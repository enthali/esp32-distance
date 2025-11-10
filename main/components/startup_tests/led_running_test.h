/**
 * @file led_running_test.h
 * @brief Running LED Test for LED Controller
 *
 * Test functions for running light effects on the LED strip.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "led_controller.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Run a single cycle of running light effect
     *
     * Creates a running light effect where one LED moves across the strip.
     *
     * @param color Color of the running LED
     * @param delay_ms Delay between LED movements in milliseconds
     * @return ESP_OK on success
     */
    esp_err_t led_running_test_single_cycle(led_color_t color, uint32_t delay_ms);

    /**
     * @brief Run multiple cycles of running light effect
     *
     * Runs the running light effect for specified number of cycles.
     *
     * @param color Color of the running LED
     * @param delay_ms Delay between LED movements in milliseconds
     * @param cycles Number of complete cycles to run
     * @return ESP_OK on success
     */
    esp_err_t led_running_test_multiple_cycles(led_color_t color, uint32_t delay_ms, uint8_t cycles);

    /**
     * @brief Run rainbow running light effect
     *
     * Running light that changes color as it moves (rainbow effect).
     *
     * @param delay_ms Delay between LED movements in milliseconds
     * @param cycles Number of complete cycles to run
     * @return ESP_OK on success
     */
    esp_err_t led_running_test_rainbow(uint32_t delay_ms, uint8_t cycles);

#ifdef __cplusplus
}
#endif
