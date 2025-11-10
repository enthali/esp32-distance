/**
 * @file led_controller_sim.c
 * @brief WS2812 LED Strip Controller Simulator for QEMU
 *
 * Provides identical API to led_controller.c but displays LED state using
 * Unicode emoji blocks in the terminal instead of controlling hardware.
 * Features rate limiting to prevent terminal spam.
 *
 * VISUALIZATION STRATEGY:
 * ======================
 * - Uses Unicode emoji blocks for color representation
 * - Rate limited to ~1 update per second maximum
 * - Color mapping based on dominant RGB channel
 * - Terminal-friendly output format
 *
 * API COMPATIBILITY:
 * ==================
 * - Identical function signatures to led_controller.c
 * - Same return codes and error handling
 * - Same buffer management and pixel operations
 * - Same color constants and utility functions
 *
 * @author ESP32 Distance Project
 * @date 2025
 */

#include "led_controller.h"
#include "config_manager.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char *TAG = "led_controller_sim";

/**
 * @brief LED strip configuration structure (PRIVATE)
 * 
 * This struct is NOT part of the public API.
 * Configuration is loaded from config_manager during led_controller_init().
 */
typedef struct
{
    gpio_num_t gpio_pin;       ///< Ignored in simulator
    uint16_t led_count;        ///< Number of LEDs in the strip
    int rmt_channel;           ///< Ignored in simulator
    uint8_t brightness;        ///< Global brightness (0-255)
} led_config_t;

// Internal state - same structure as hardware version
static led_color_t *led_buffer = NULL;
static led_config_t current_config = {0};
static bool is_initialized = false;

// Rate limiting for display updates (prevent terminal spam)
static uint64_t last_display_time = 0;
static const uint64_t DISPLAY_INTERVAL_US = 250000; // 250 ms

// Optional status text appended to simulated display (small buffer)
static char status_text[64] = "";

// Predefined color constants - same as hardware
const led_color_t LED_COLOR_RED = {255, 0, 0};
const led_color_t LED_COLOR_GREEN = {0, 255, 0};
const led_color_t LED_COLOR_BLUE = {0, 0, 255};
const led_color_t LED_COLOR_WHITE = {255, 255, 255};
const led_color_t LED_COLOR_YELLOW = {255, 255, 0};
const led_color_t LED_COLOR_ORANGE = {255, 165, 0};
const led_color_t LED_COLOR_CYAN = {0, 255, 255};
const led_color_t LED_COLOR_MAGENTA = {255, 0, 255};
const led_color_t LED_COLOR_OFF = {0, 0, 0};

/**
 * @brief Map RGB color to appropriate emoji block
 * 
 * Uses color analysis to select the most representative emoji for display.
 */
static const char* color_to_emoji(led_color_t color)
{
    // Calculate total brightness
    uint16_t total_brightness = color.red + color.green + color.blue;
    
    // Handle off/very dim pixels
    if (total_brightness < 30) {
        return "⚫"; // Black/off
    }
    
    // Handle pure or near-pure colors (single channel dominant)
    if (color.red > 200 && color.green < 50 && color.blue < 50) {
        return "🔴"; // Red
    } else if (color.green > 200 && color.red < 50 && color.blue < 50) {
        return "🟢"; // Green
    } else if (color.blue > 200 && color.red < 50 && color.green < 50) {
        return "🔵"; // Blue
    }
    
    // Handle mixed colors
    if (color.red > 150 && color.blue > 150 && color.green < 100) {
        return "🟣"; // Purple/Magenta
    } else if (color.red > 200 && color.green > 100 && color.green < 200 && color.blue < 50) {
        return "🟠"; // Orange
    } else if (color.red > 150 && color.green > 150 && color.blue < 100) {
        return "🟡"; // Yellow
    } else if (color.green > 150 && color.blue > 150 && color.red < 100) {
        return "🔷"; // Cyan-like (diamond for variety)
    }
    
    // Handle bright white/mixed colors
    if (total_brightness > 600) {
        return "⚪"; // White/bright
    }
    
    // Default for other mixed/dim colors
    return "🟤"; // Brown/mixed
}

esp_err_t led_controller_init(gpio_num_t data_pin)
{
    if (is_initialized)
    {
        ESP_LOGW(TAG, "LED controller simulator already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing LED controller simulator (loading config from NVS)...");
    
    // Store hardware pin (ignored in simulator but logged for consistency)
    current_config.gpio_pin = data_pin;
    current_config.rmt_channel = 0;
    
    ESP_LOGI(TAG, "Hardware: GPIO%d (ignored in simulator)", data_pin);
    
    // Load LED count from config_manager
    int32_t led_count = 0;
    esp_err_t ret = config_get_int32("led_count", &led_count);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read led_count from config: %s", esp_err_to_name(ret));
        return ret;
    }
    
    if (led_count <= 0 || led_count > 1000) {
        ESP_LOGE(TAG, "Invalid LED count from config: %ld", led_count);
        return ESP_ERR_INVALID_ARG;
    }
    current_config.led_count = (uint16_t)led_count;
    
    // Load brightness from config_manager (ignored in simulator but logged)
    int32_t led_bright = 0;
    config_get_int32("led_bright", &led_bright);
    current_config.brightness = (uint8_t)led_bright;
    
    ESP_LOGI(TAG, "Configuration loaded from NVS:");
    ESP_LOGI(TAG, "  LED Count: %d", current_config.led_count);
    ESP_LOGI(TAG, "  [Simulator: brightness, GPIO, RMT ignored]");

    // Allocate LED buffer
    led_buffer = malloc(current_config.led_count * sizeof(led_color_t));
    if (led_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return ESP_ERR_NO_MEM;
    }

    // Initialize all LEDs to off
    for (uint16_t i = 0; i < current_config.led_count; i++)
    {
        led_buffer[i] = LED_COLOR_OFF;
    }

    is_initialized = true;
    ESP_LOGI(TAG, "LED controller simulator initialized: %d LEDs (terminal visualization)", 
             current_config.led_count);
    
    return ESP_OK;
}

esp_err_t led_controller_deinit(void)
{
    if (!is_initialized)
    {
        ESP_LOGW(TAG, "LED controller simulator not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Free LED buffer
    if (led_buffer != NULL)
    {
        free(led_buffer);
        led_buffer = NULL;
    }

    // Clear configuration
    memset(&current_config, 0, sizeof(current_config));
    is_initialized = false;
    
    ESP_LOGI(TAG, "LED controller simulator deinitialized");
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

    // Rate limiting: only display ~1Hz to prevent terminal spam
    uint64_t now = esp_timer_get_time();
    if (now - last_display_time < DISPLAY_INTERVAL_US) {
        return ESP_OK;  // Suppress output, just return success
    }
    last_display_time = now;

    // Build LED strip visualization string
    char output[1024];  // Large enough for 40 LEDs + text
    int pos = 0;
    
    pos += sprintf(output + pos, "[LED Strip]: ");
    
    for (uint16_t i = 0; i < current_config.led_count; i++) {
        const char* emoji = color_to_emoji(led_buffer[i]);
        pos += sprintf(output + pos, "%s", emoji);
        
        // Safety check to prevent buffer overflow
        if (pos > sizeof(output) - 100) break;
    }

    // Append current measurement (if available) to the end of the line
    if (status_text[0] != '\0') {
        pos += sprintf(output + pos, "  %s", status_text);
    }
    
    // Output to console (stdout)
    printf("%s\r", output);
    fflush(stdout);
    
    return ESP_OK;
}

esp_err_t led_controller_set_status_text(const char *text)
{
    if (text == NULL) {
        status_text[0] = '\0';
        return ESP_OK;
    }

    // Truncate to buffer size - 1
    strncpy(status_text, text, sizeof(status_text) - 1);
    status_text[sizeof(status_text) - 1] = '\0';
    return ESP_OK;
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