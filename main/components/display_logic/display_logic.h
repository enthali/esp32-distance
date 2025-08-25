/**
 * @file display_logic.h
 * @brief LED Distance Visualization Business Logic Component
 *
 * Business logic component that converts distance measurements into LED strip visualizations.
 * Provides clean separation between hardware abstraction (components) and application logic.
 *
 * FEATURES:
 * =========
 * - Distance Range Mapping: Configurable range mapped linearly to configured LED count
 * - LED Spacing: Approximately (max_distance - min_distance) / led_count cm per LED
 * - Visual Feedback:
 *   - Normal range: Green/blue gradient or solid color
 *   - Below 10cm: First LED red (error indicator)
 *   - Above 50cm: Last LED red (error indicator)
 *   - Sensor timeout/error: All LEDs off or specific error pattern
 *
 * ARCHITECTURE:
 * =============
 * - Priority 3 Task: LED visualization (between sensor priority 6 and test priority 2)
 * - Blocking reads from distance sensor queue (waits for new measurements)
 * - Update rate matches distance sensor measurement rate
 * - Real-time safe with proper task priorities
 *
 * INTEGRATION:
 * ============
 * - Input: distance_sensor_get_latest() API (blocking - waits for new data)
 * - Output: led_controller APIs (led_set_pixel, led_clear_all, led_show)
 * - Error handling: Visual indicators for all sensor error states
 *
 * @author ESP32 Distance Project
 * @date 2025
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Configuration structure removed - now uses config_manager API directly

    /**
     * @brief Initialize the display logic component
     *
     * Initializes the display logic. Configuration is obtained from config_manager.
     * Must be called before display_logic_start().
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     *
     * @note Configuration obtained from config_manager API (REQ-CFG-2)
     */
    esp_err_t display_logic_init(void);

    /**
     * @brief Start the display logic task
     *
     * Creates and starts the display logic task with priority 3.
     * Task will continuously wait for distance measurements and update LED strip.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     *
     * @note Task runs on core 1 with 4KB stack at priority 3
     * @note display_logic_init() must be called first
     * @note Task blocks on distance_sensor_get_latest() until new data arrives
     */
    esp_err_t display_logic_start(void);

    /**
     * @brief Stop the display logic task
     *
     * Stops the display logic task and clears all LEDs.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     */
    esp_err_t display_logic_stop(void);

    /**
     * @brief Check if display logic task is running
     *
     * @return true if task is running, false otherwise
     */
    bool display_logic_is_running(void);

    // display_logic_get_config() function removed - configuration access via config_manager API

#ifdef __cplusplus
}
#endif