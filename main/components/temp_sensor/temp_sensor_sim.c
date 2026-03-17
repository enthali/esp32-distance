/**
 * @file temp_sensor_sim.c
 * @brief DS18B20 Temperature Sensor — QEMU Simulation
 *
 * Compiled instead of temp_sensor.c when CONFIG_TEMP_SENSOR_SIMULATION=y.
 * Sweeps temperature from 0 °C to 25 °C over ~30 seconds (2-second cycles),
 * then resets to 0 °C, allowing visual verification of the full
 * blue→green→orange LED colour gradient in QEMU.
 *
 * No 1-Wire GPIO activity in simulation mode.
 *
 * DESIGN TRACEABILITY: SPEC_TEMP_ARCH_1 (QEMU Simulation section)
 */

#include "temp_sensor.h"
#include "distance_sensor.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "temp_sensor_sim";

/* Simulation sweeps 0 °C → 25 °C in 15 × 2-second steps (~1.667 °C/step).
 * Expressed in 0.1 °C units: 0 → 250, step ≈ 17 (rounds to 17). */
#define SIM_TEMP_START_C10   0
#define SIM_TEMP_END_C10   250
#define SIM_STEP_C10        17   /* ≈ 250 / 15 */

static temp_sensor_config_t s_config = {
    .data_pin    = GPIO_NUM_4,
    .interval_ms = 2000,
};
static bool               s_initialized  = false;
static TaskHandle_t       s_task_handle  = NULL;
static SemaphoreHandle_t  s_mutex        = NULL;
static temp_measurement_t s_latest       = { .valid = false };

static void temp_sensor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Temperature sensor simulator started (sweep 0→25°C, step %d/10°C, interval %lu ms)",
             SIM_STEP_C10, (unsigned long)s_config.interval_ms);

    int16_t sim_temp_c_x10 = SIM_TEMP_START_C10;

    /* Tell distance sensor to start with default 20 °C */
    distance_sensor_set_temperature(200);

    while (1) {
        /* Update simulated temperature */
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        s_latest.temperature_c_x10 = sim_temp_c_x10;
        s_latest.valid              = true;
        s_latest.timestamp_us       = esp_timer_get_time();
        xSemaphoreGive(s_mutex);

        /* Push to distance sensor */
        distance_sensor_set_temperature(sim_temp_c_x10);

        ESP_LOGD(TAG, "Simulated temperature: %d.%d °C",
                 sim_temp_c_x10 / 10, sim_temp_c_x10 % 10);

        /* Advance sweep */
        sim_temp_c_x10 += SIM_STEP_C10;
        if (sim_temp_c_x10 > SIM_TEMP_END_C10) {
            sim_temp_c_x10 = SIM_TEMP_START_C10; /* reset to start */
        }

        vTaskDelay(pdMS_TO_TICKS(s_config.interval_ms));
    }
}

esp_err_t temp_sensor_init(const temp_sensor_config_t *config)
{
    if (config != NULL) {
        s_config = *config;
    } else {
        int32_t gpio_num = 4;
        int32_t intv_ms  = 2000;
        config_get_int32("temp_gpio",        &gpio_num);
        config_get_int32("temp_interval_ms", &intv_ms);
        s_config.data_pin    = (gpio_num_t)gpio_num;
        s_config.interval_ms = (uint32_t)intv_ms;
    }

    s_mutex = xSemaphoreCreateMutex();
    if (s_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_ERR_NO_MEM;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "Temperature sensor simulator initialized");
    return ESP_OK;
}

esp_err_t temp_sensor_start(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_task_handle != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        temp_sensor_task,
        "temp_sensor_sim",
        2048,
        NULL,
        3,
        &s_task_handle,
        0
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create simulator task");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Temperature sensor simulator task started");
    return ESP_OK;
}

esp_err_t temp_sensor_stop(void)
{
    if (s_task_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    vTaskDelete(s_task_handle);
    s_task_handle = NULL;
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
