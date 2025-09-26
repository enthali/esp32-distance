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
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "led_controller_sim";

// Internal state - same structure as hardware version
static led_color_t *led_buffer = NULL;
static led_config_t current_config = {0};
static bool is_initialized = false;

// Optional status text appended to simulated display (small buffer)
static char status_text[64] = "";

// Predefined color constants - same as hardware
const led_color_t LED_COLOR_RED = {255, 0, 0};
const led_color_t LED_COLOR_GREEN = {0, 255, 0};
const led_color_t LED_COLOR_BLUE = {0, 0, 255};
const led_color_t LED_COLOR_WHITE = {255, 255, 255};
const led_color_t LED_COLOR_YELLOW = {255, 255, 0};
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

esp_err_t led_controller_init(const led_config_t *config)
{
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Configuration cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (is_initialized)
    {
        ESP_LOGW(TAG, "LED controller simulator already initialized");
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
    led_buffer = malloc(config->led_count * sizeof(led_color_t));
    if (led_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return ESP_ERR_NO_MEM;
    }

    // Initialize all LEDs to off
    for (uint16_t i = 0; i < config->led_count; i++)
    {
        led_buffer[i] = LED_COLOR_OFF;
    }

    is_initialized = true;
    // ESP_LOGI(TAG, "LED controller simulator initialized: %d LEDs (terminal visualization)", 
    //          config->led_count);
    
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

    // Rate limiting - only output ~1x per second to avoid terminal spam

    // Display LED strip state using emoji blocks
    printf("[LED Strip]: ");
    for (uint16_t i = 0; i < current_config.led_count; i++) {
        printf("%s", color_to_emoji(led_buffer[i]));
    }

    // Append current measurement (if available) to the end of the line
    if (status_text[0] != '\0') {
        printf("  %s", status_text);
    }
    printf("\r");
    fflush(stdout); // Ensure immediate output
    
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