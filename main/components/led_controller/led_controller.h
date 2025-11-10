/**
 * @file led_controller.h
 * @brief WS2812 LED Strip Controller for ESP32
 *
 * Hardware abstraction layer for WS2812 addressable LED strips using ESP32 RMT peripheral.
 * Provides low-level control with RAM buffer management for efficient updates.
 *
 * Features:
 * - Individual pixel control (set/get/clear)
 * - RAM buffer maintains current LED state
 * - Manual update trigger for performance optimization
 * - Configurable LED count and GPIO pin
 * - Color utility functions
 *
 * @author ESP32 Distance Project
 * @date 2025
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/rmt_tx.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief RGB Color structure
     *
     * Represents a color in RGB color space with 8-bit resolution per channel.
     */
    typedef struct
    {
        uint8_t red;   ///< Red component (0-255)
        uint8_t green; ///< Green component (0-255)
        uint8_t blue;  ///< Blue component (0-255)
    } led_color_t;

    /**
     * @brief LED strip configuration structure
     *
     * Configuration parameters for initializing the LED controller.
     */
    typedef struct
    {
        gpio_num_t gpio_pin;       ///< GPIO pin for WS2812 data line
        uint16_t led_count;        ///< Number of LEDs in the strip
        int rmt_channel;           ///< RMT channel number (0 to 7)
    } led_config_t;

    /**
     * @brief Predefined color constants
     *
     * Common colors for convenience. Use led_color_rgb() for custom colors.
     */
    extern const led_color_t LED_COLOR_RED;     ///< Pure red (255,0,0)
    extern const led_color_t LED_COLOR_GREEN;   ///< Pure green (0,255,0)
    extern const led_color_t LED_COLOR_BLUE;    ///< Pure blue (0,0,255)
    extern const led_color_t LED_COLOR_WHITE;   ///< Pure white (255,255,255)
    extern const led_color_t LED_COLOR_YELLOW;  ///< Yellow (255,255,0)
    extern const led_color_t LED_COLOR_CYAN;    ///< Cyan (0,255,255)
    extern const led_color_t LED_COLOR_MAGENTA; ///< Magenta (255,0,255)
    extern const led_color_t LED_COLOR_OFF;     ///< Off/Black (0,0,0)

    /**
     * @brief Initialize the LED controller
     *
     * Initializes the RMT peripheral, allocates RAM buffer, and configures GPIO.
     * Must be called before any other LED controller functions.
     *
     * @param config Pointer to LED strip configuration
     * @return ESP_OK on success, ESP_ERR_* on failure
     *
     * @note This function allocates memory for the LED buffer (3 bytes per LED)
     * @note Only one LED controller instance is supported
     */
    esp_err_t led_controller_init(const led_config_t *config);

    /**
     * @brief Deinitialize the LED controller
     *
     * Frees allocated memory, deinitializes RMT peripheral, and resets GPIO.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     */
    esp_err_t led_controller_deinit(void);

    /**
     * @brief Set color of a specific LED pixel
     *
     * Updates the LED color in RAM buffer. Call led_show() to update physical LEDs.
     *
     * @param index LED index (0 to led_count-1)
     * @param color RGB color to set
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if index out of range
     */
    esp_err_t led_set_pixel(uint16_t index, led_color_t color);

    /**
     * @brief Clear a specific LED pixel (turn off)
     *
     * Sets the LED to off state (black). Equivalent to led_set_pixel(index, LED_COLOR_OFF).
     *
     * @param index LED index (0 to led_count-1)
     * @return ESP_OK on success, ESP_ERR_INVALID_ARG if index out of range
     */
    esp_err_t led_clear_pixel(uint16_t index);

    /**
     * @brief Get current color of a specific LED pixel
     *
     * Reads the current color from RAM buffer (not from physical LED).
     *
     * @param index LED index (0 to led_count-1)
     * @return Current LED color, or LED_COLOR_OFF if index out of range
     */
    led_color_t led_get_pixel(uint16_t index);

    /**
     * @brief Clear all LED pixels (turn off all LEDs)
     *
     * Sets all LEDs to off state in RAM buffer. Call led_show() to update physical LEDs.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     */
    esp_err_t led_clear_all(void);

    /**
     * @brief Update physical LED strip with current RAM buffer
     *
     * Transmits the current LED state from RAM buffer to the physical LED strip
     * using the RMT peripheral. This is the only function that actually updates
     * the visible LEDs.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     *
     * @note This function blocks until transmission is complete (~1-2ms for 40 LEDs)
     */
    esp_err_t led_show(void);

    /**
     * @brief Create RGB color from individual components
     *
     * Utility function to create led_color_t structure from RGB values.
     *
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @return RGB color structure
     */
    led_color_t led_color_rgb(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Apply brightness scaling to a color
     *
     * Scales all RGB components by brightness factor while maintaining color ratio.
     *
     * @param color Original color
     * @param brightness Brightness factor (0=off, 255=full brightness)
     * @return Color with applied brightness scaling
     *
     * @note Uses integer math to avoid floating point operations
     */
    led_color_t led_color_brightness(led_color_t color, uint8_t brightness);

    /**
     * @brief Get configured LED count
     *
     * Returns the number of LEDs configured during initialization.
     *
     * @return Number of LEDs, or 0 if not initialized
     */
    uint16_t led_get_count(void);

    /**
     * @brief Check if LED controller is initialized
     *
     * @return true if initialized, false otherwise
     */
    bool led_is_initialized(void);

#ifdef __cplusplus
}
#endif
