/**
 * @file temp_sensor.c
 * @brief DS18B20 Dallas 1-Wire Temperature Sensor — Hardware Implementation
 *
 * Implements Dallas 1-Wire bit-banging via GPIO direction toggling.
 * A single FreeRTOS task drives the convert→wait→read sequence and pushes
 * each successful reading to a mutex-protected shared buffer.  Consumers
 * call temp_sensor_get_latest() from any task without blocking.
 *
 * After each successful measurement the distance sensor compensation value
 * is updated via distance_sensor_set_temperature() (REQ_TEMP_5).
 *
 * REQUIREMENTS TRACEABILITY:
 *   REQ_TEMP_1, REQ_TEMP_2, REQ_TEMP_3, REQ_TEMP_4, REQ_TEMP_5
 *
 * DESIGN TRACEABILITY:
 *   SPEC_TEMP_ARCH_1, SPEC_TEMP_ONEWIRE_1, SPEC_TEMP_TASK_1, SPEC_TEMP_API_1
 */

#include "temp_sensor.h"
#include "distance_sensor.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

static const char *TAG = "temp_sensor";

/* -------------------------------------------------------------------------
 * Module state
 * ---------------------------------------------------------------------- */
static temp_sensor_config_t s_config = {
    .data_pin    = GPIO_NUM_4,
    .interval_ms = 2000,
};
static bool              s_initialized         = false;
static TaskHandle_t      s_task_handle         = NULL;
static SemaphoreHandle_t s_mutex               = NULL;
static temp_measurement_t s_latest             = { .valid = false };
static int               s_consecutive_errors  = 0;

/* -------------------------------------------------------------------------
 * 1-Wire helpers (SPEC_TEMP_ONEWIRE_1)
 * ---------------------------------------------------------------------- */

/** Pull line LOW (open-drain: configure as push-pull output low) */
static inline void ow_pull_low(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
}

/** Release line (high-impedance input; internal/external pull-up takes it HIGH) */
static inline void ow_release(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}

/**
 * @brief Issue a 1-Wire reset and detect presence pulse.
 * @return true if a slave responded with a presence pulse.
 */
static bool ow_reset(gpio_num_t pin)
{
    portDISABLE_INTERRUPTS();
    ow_pull_low(pin);
    esp_rom_delay_us(480);          /* ≥480 µs reset pulse */
    ow_release(pin);
    esp_rom_delay_us(70);           /* wait for presence window */
    bool presence = (gpio_get_level(pin) == 0); /* DS18B20 pulls LOW */
    esp_rom_delay_us(410);          /* complete slot (480 µs total after release) */
    portENABLE_INTERRUPTS();
    return presence;
}

/** Write one bit onto the 1-Wire bus. */
static void ow_write_bit(gpio_num_t pin, uint8_t bit)
{
    portDISABLE_INTERRUPTS();
    if (bit) {
        ow_pull_low(pin);
        esp_rom_delay_us(6);
        ow_release(pin);
        esp_rom_delay_us(64);
    } else {
        ow_pull_low(pin);
        esp_rom_delay_us(60);
        ow_release(pin);
        esp_rom_delay_us(10);
    }
    portENABLE_INTERRUPTS();
}

/** Write one byte onto the 1-Wire bus (LSB first). */
static void ow_write_byte(gpio_num_t pin, uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        ow_write_bit(pin, byte & 0x01);
        byte >>= 1;
    }
}

/** Read one bit from the 1-Wire bus. */
static uint8_t ow_read_bit(gpio_num_t pin)
{
    portDISABLE_INTERRUPTS();
    ow_pull_low(pin);
    esp_rom_delay_us(6);
    ow_release(pin);
    esp_rom_delay_us(9);
    uint8_t bit = (uint8_t)gpio_get_level(pin);
    esp_rom_delay_us(55);
    portENABLE_INTERRUPTS();
    return bit;
}

/** Read one byte from the 1-Wire bus (LSB first). */
static uint8_t ow_read_byte(gpio_num_t pin)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte |= (uint8_t)(ow_read_bit(pin) << i);
    }
    return byte;
}

/* -------------------------------------------------------------------------
 * DS18B20 helpers
 * ---------------------------------------------------------------------- */

/**
 * @brief Compute Dallas/Maxim CRC-8 (poly 0x31).
 * @return CRC byte that should match scratchpad[8].
 */
static uint8_t ds18b20_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc ^ byte) & 0x01) {
                crc = (crc >> 1) ^ 0x8C;
            } else {
                crc >>= 1;
            }
            byte >>= 1;
        }
    }
    return crc;
}

/**
 * @brief Decode temperature from DS18B20 scratchpad (12-bit resolution).
 *
 * Raw value is a 16-bit little-endian two's-complement number where
 * 1 LSB = 0.0625 °C.  Multiply by 10, divide by 16 → 0.1 °C units.
 *
 * @param scratchpad  9-byte scratchpad read from the sensor.
 * @return Temperature in 0.1 °C units (e.g. 215 = 21.5 °C).
 */
static int16_t ds18b20_decode_temp(const uint8_t *scratchpad)
{
    int16_t raw = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);
    return (int16_t)((raw * 10) / 16);
}

/* -------------------------------------------------------------------------
 * Measurement task (SPEC_TEMP_TASK_1)
 * ---------------------------------------------------------------------- */

static void temp_sensor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Temperature sensor task started (pin GPIO%d, interval %lu ms)",
             (int)s_config.data_pin, (unsigned long)s_config.interval_ms);

    /* On startup tell distance sensor to use default 20 °C (REQ_TEMP_5 AC-2) */
    distance_sensor_set_temperature(200);

    TickType_t interval_ticks = pdMS_TO_TICKS(s_config.interval_ms);

    while (1) {
        TickType_t tick_start = xTaskGetTickCount();
        bool success = false;

        /* Step 1: Initiate temperature conversion */
        if (!ow_reset(s_config.data_pin)) {
            ESP_LOGW(TAG, "DS18B20 not detected — no presence pulse");
            goto sleep;
        }
        ow_write_byte(s_config.data_pin, 0xCC); /* Skip ROM */
        ow_write_byte(s_config.data_pin, 0x44); /* Convert T */

        /* Step 2: Wait ≥750 ms for 12-bit conversion (REQ_TEMP_3 AC-3) */
        vTaskDelay(pdMS_TO_TICKS(800));

        /* Step 3: Read scratchpad */
        if (!ow_reset(s_config.data_pin)) {
            ESP_LOGW(TAG, "DS18B20 disappeared after conversion");
            goto sleep;
        }
        ow_write_byte(s_config.data_pin, 0xCC); /* Skip ROM */
        ow_write_byte(s_config.data_pin, 0xBE); /* Read Scratchpad */

        uint8_t scratchpad[9];
        for (int i = 0; i < 9; i++) {
            scratchpad[i] = ow_read_byte(s_config.data_pin);
        }

        /* Step 4: Validate CRC */
        if (ds18b20_crc8(scratchpad, 8) != scratchpad[8]) {
            ESP_LOGW(TAG, "DS18B20 CRC mismatch — retaining last valid value");
            goto sleep;
        }

        /* Step 5: Decode and store */
        int16_t temp_c_x10 = ds18b20_decode_temp(scratchpad);

        xSemaphoreTake(s_mutex, portMAX_DELAY);
        s_latest.temperature_c_x10 = temp_c_x10;
        s_latest.valid              = true;
        s_latest.timestamp_us       = esp_timer_get_time();
        xSemaphoreGive(s_mutex);

        /* Step 6: Update distance sensor compensation (REQ_TEMP_5 AC-1) */
        distance_sensor_set_temperature(temp_c_x10);

        s_consecutive_errors = 0;
        success = true;
        ESP_LOGI(TAG, "Temperature: %d.%d °C",
                 temp_c_x10 / 10, (temp_c_x10 < 0 ? -(temp_c_x10 % 10) : temp_c_x10 % 10));

    sleep:
        if (!success) {
            s_consecutive_errors++;
            if (s_consecutive_errors >= 5) {
                ESP_LOGE(TAG, "DS18B20: %d consecutive failures — sensor missing or wiring fault",
                         s_consecutive_errors);
            }
        }
        TickType_t elapsed = xTaskGetTickCount() - tick_start;
        TickType_t remaining = (interval_ticks > elapsed) ? (interval_ticks - elapsed) : 0;
        if (remaining > 0) {
            vTaskDelay(remaining);
        }
    }
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

esp_err_t temp_sensor_init(const temp_sensor_config_t *config)
{
    if (config != NULL) {
        s_config = *config;
    } else {
        /* Load from config_manager (REQ_TEMP_1 AC-1, SPEC_TEMP_API_1) */
        int32_t gpio_num  = 4;
        int32_t intv_ms   = 2000;
        config_get_int32("temp_gpio",        &gpio_num);
        config_get_int32("temp_intv_ms", &intv_ms);
        s_config.data_pin    = (gpio_num_t)gpio_num;
        s_config.interval_ms = (uint32_t)intv_ms;
    }

    /* Configure data pin as input with pull-up (open-drain idle state) */
    gpio_config_t io_cfg = {
        .pin_bit_mask = (1ULL << s_config.data_pin),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,  /* internal weak pull-up backup */
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure data pin GPIO%d: %s",
                 (int)s_config.data_pin, esp_err_to_name(ret));
        return ret;
    }

    s_mutex = xSemaphoreCreateMutex();
    if (s_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_ERR_NO_MEM;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "Temperature sensor initialized (pin GPIO%d, interval %lu ms)",
             (int)s_config.data_pin, (unsigned long)s_config.interval_ms);
    return ESP_OK;
}

esp_err_t temp_sensor_start(void)
{
    if (!s_initialized) {
        ESP_LOGE(TAG, "Not initialized — call temp_sensor_init() first");
        return ESP_ERR_INVALID_STATE;
    }
    if (s_task_handle != NULL) {
        ESP_LOGW(TAG, "Task already running");
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        temp_sensor_task,
        "temp_sensor",
        2048,           /* stack bytes — REQ_TEMP_3 AC-5 */
        NULL,
        3,              /* priority — same as display_logic */
        &s_task_handle,
        0               /* core 0 */
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Temperature sensor task started");
    return ESP_OK;
}

esp_err_t temp_sensor_stop(void)
{
    if (s_task_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    vTaskDelete(s_task_handle);
    s_task_handle = NULL;
    ESP_LOGI(TAG, "Temperature sensor task stopped");
    return ESP_OK;
}

esp_err_t temp_sensor_get_latest(temp_measurement_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_mutex == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    *out = s_latest;
    xSemaphoreGive(s_mutex);

    return s_latest.valid ? ESP_OK : ESP_ERR_INVALID_STATE;
}
