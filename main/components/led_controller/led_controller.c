/**
 * @file led_controller.c
 * @brief WS2812 LED Strip Controller Implementation
 *
 * Implementation of WS2812 LED controller using ESP32 RMT peripheral.
 * Manages LED state in RAM buffer and provides hardware abstraction.
 */

#include "led_controller.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "led_controller";

// WS2812 timing constants (in RMT ticks at 80MHz)
#define WS2812_T0H_TICKS 32     // 0.4us high for bit 0
#define WS2812_T0L_TICKS 64     // 0.8us low for bit 0
#define WS2812_T1H_TICKS 64     // 0.8us high for bit 1
#define WS2812_T1L_TICKS 32     // 0.4us low for bit 1
#define WS2812_RESET_TICKS 4000 // 50us reset pulse

// Internal state
static led_color_t *led_buffer = NULL;
static led_config_t current_config = {0};
static bool is_initialized = false;
static rmt_channel_handle_t rmt_channel_handle = NULL;
static rmt_encoder_handle_t rmt_encoder_handle = NULL;
static rmt_transmit_config_t rmt_tx_config = {0};

// Predefined color constants
const led_color_t LED_COLOR_RED = {255, 0, 0};
const led_color_t LED_COLOR_GREEN = {0, 255, 0};
const led_color_t LED_COLOR_BLUE = {0, 0, 255};
const led_color_t LED_COLOR_WHITE = {255, 255, 255};
const led_color_t LED_COLOR_YELLOW = {255, 255, 0};
const led_color_t LED_COLOR_CYAN = {0, 255, 255};
const led_color_t LED_COLOR_MAGENTA = {255, 0, 255};
const led_color_t LED_COLOR_OFF = {0, 0, 0};

/**
 * @brief Create WS2812 RMT encoder
 */
static esp_err_t rmt_new_led_strip_encoder(rmt_encoder_handle_t *ret_encoder)
{
    // Create bytes encoder for RGB data
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = WS2812_T0H_TICKS,
            .level1 = 0,
            .duration1 = WS2812_T0L_TICKS,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = WS2812_T1H_TICKS,
            .level1 = 0,
            .duration1 = WS2812_T1L_TICKS,
        },
        .flags.msb_first = 1,
    };
    
    return rmt_new_bytes_encoder(&bytes_encoder_config, ret_encoder);
}

/**
 * @brief Configure RMT channel for WS2812 timing
 */
static esp_err_t configure_rmt_channel(void)
{
    // Configure RMT TX channel
    rmt_tx_channel_config_t tx_channel_config = {
        .gpio_num = current_config.gpio_pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 80000000, // 80MHz resolution
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };

    esp_err_t ret = rmt_new_tx_channel(&tx_channel_config, &rmt_channel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create LED strip encoder
    ret = rmt_new_led_strip_encoder(&rmt_encoder_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip encoder: %s", esp_err_to_name(ret));
        rmt_del_channel(rmt_channel_handle);
        rmt_channel_handle = NULL;
        return ret;
    }

    // Enable RMT TX channel
    ret = rmt_enable(rmt_channel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable RMT channel: %s", esp_err_to_name(ret));
        rmt_del_encoder(rmt_encoder_handle);
        rmt_del_channel(rmt_channel_handle);
        rmt_channel_handle = NULL;
        rmt_encoder_handle = NULL;
        return ret;
    }

    // Configure transmission
    rmt_tx_config.loop_count = 0; // No loop

    return ESP_OK;
}

esp_err_t led_controller_init(const led_config_t *config)
{
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Configuration cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (is_initialized)
    {
        ESP_LOGW(TAG, "LED controller already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (config->led_count == 0 || config->led_count > 1000)
    {
        ESP_LOGE(TAG, "Invalid LED count: %d", config->led_count);
        return ESP_ERR_INVALID_ARG;
    }

    // Store configuration
    current_config = *config;

    // Allocate LED buffer
    led_buffer = (led_color_t *)malloc(config->led_count * sizeof(led_color_t));
    if (led_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return ESP_ERR_NO_MEM;
    }

    // Configure RMT
    esp_err_t ret = configure_rmt_channel();
    if (ret != ESP_OK)
    {
        free(led_buffer);
        led_buffer = NULL;
        return ret;
    }

    // Initialize all LEDs to off
    led_clear_all();

    is_initialized = true;
    ESP_LOGI(TAG, "LED controller initialized: %d LEDs on GPIO%d, RMT channel %d",
             config->led_count, config->gpio_pin, config->rmt_channel);

    return ESP_OK;
}

esp_err_t led_controller_deinit(void)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // Turn off all LEDs
    led_clear_all();
    led_show();

    // Cleanup RMT
    if (rmt_channel_handle) {
        rmt_disable(rmt_channel_handle);
        rmt_del_channel(rmt_channel_handle);
        rmt_channel_handle = NULL;
    }
    if (rmt_encoder_handle) {
        rmt_del_encoder(rmt_encoder_handle);
        rmt_encoder_handle = NULL;
    }

    // Free memory
    free(led_buffer);
    led_buffer = NULL;

    is_initialized = false;
    ESP_LOGI(TAG, "LED controller deinitialized");

    return ESP_OK;
}

esp_err_t led_set_pixel(uint16_t index, led_color_t color)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (index >= current_config.led_count)
    {
        ESP_LOGE(TAG, "LED index %d out of range (0-%d)", index, current_config.led_count - 1);
        return ESP_ERR_INVALID_ARG;
    }

    led_buffer[index] = color;
    return ESP_OK;
}

esp_err_t led_clear_pixel(uint16_t index)
{
    return led_set_pixel(index, LED_COLOR_OFF);
}

led_color_t led_get_pixel(uint16_t index)
{
    if (!is_initialized || index >= current_config.led_count)
    {
        return LED_COLOR_OFF;
    }

    return led_buffer[index];
}

esp_err_t led_clear_all(void)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    for (uint16_t i = 0; i < current_config.led_count; i++)
    {
        led_buffer[i] = LED_COLOR_OFF;
    }

    return ESP_OK;
}

esp_err_t led_show(void)
{
    if (!is_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    // Prepare data buffer in GRB order for WS2812
    size_t data_size = current_config.led_count * 3; // 3 bytes per LED (GRB)
    uint8_t *data_buffer = malloc(data_size);
    if (!data_buffer) {
        ESP_LOGE(TAG, "Failed to allocate data buffer");
        return ESP_ERR_NO_MEM;
    }

    // Convert LED buffer to GRB format
    for (uint16_t led = 0; led < current_config.led_count; led++)
    {
        data_buffer[led * 3 + 0] = led_buffer[led].green; // G
        data_buffer[led * 3 + 1] = led_buffer[led].red;   // R
        data_buffer[led * 3 + 2] = led_buffer[led].blue;  // B
    }

    // Transmit data using new RMT TX API
    esp_err_t ret = rmt_transmit(rmt_channel_handle, rmt_encoder_handle, 
                                data_buffer, data_size, &rmt_tx_config);
    if (ret == ESP_OK) {
        // Wait for transmission to complete
        ret = rmt_tx_wait_all_done(rmt_channel_handle, 100); // 100ms timeout
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to wait for transmission completion: %s", esp_err_to_name(ret));
        }
    } else {
        ESP_LOGE(TAG, "Failed to transmit LED data: %s", esp_err_to_name(ret));
    }

    free(data_buffer);
    return ret;
}

led_color_t led_color_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    led_color_t color = {r, g, b};
    return color;
}

led_color_t led_color_brightness(led_color_t color, uint8_t brightness)
{
    led_color_t result;
    result.red = (color.red * brightness) / 255;
    result.green = (color.green * brightness) / 255;
    result.blue = (color.blue * brightness) / 255;
    return result;
}

uint16_t led_get_count(void)
{
    return is_initialized ? current_config.led_count : 0;
}

bool led_is_initialized(void)
{
    return is_initialized;
}
