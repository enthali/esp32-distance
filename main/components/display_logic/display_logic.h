/**
 * @file display_logic.h
 * @brief LED Distance Visualization Business Logic Component
 *
 * Business logic component that converts distance measurements into LED strip visualizations.
 * Provides clean separation between hardware abstraction (components) and application logic.
 *
 * REQUIREMENTS TRACEABILITY:
 *   - REQ_DSP_1: Hardware platform and LED controller integration
 *   - REQ_DSP_2: Configuration integration (all parameters from config_manager)
 *   - REQ_DSP_3: Core visualization concept (single green LED position, real-time updates)
 *   - REQ_DSP_4: Below minimum distance warning (red LED at position 0)
 *   - REQ_DSP_5: Out of range display (red LED at last position)
 *
 * DESIGN TRACEABILITY:
 *   - SPEC_DSP_OVERVIEW_1: WS2812 hardware integration
 *   - SPEC_DSP_ARCH_1: Task-based architecture (FreeRTOS, blocking on sensor)
 *   - SPEC_DSP_ARCH_2: Configuration integration via config_manager
 *   - SPEC_DSP_ALGO_1: Distance-to-visual mapping algorithm
 *   - SPEC_DSP_ALGO_3: Embedded arithmetic architecture (integer math)
 *
 * FEATURES:
 * =========
 * - Distance Range Mapping: Configurable range mapped linearly to configured LED count
 * - LED Spacing: Approximately (dist_max_mm - dist_min_mm) / led_count mm per LED
 * - Visual Feedback:
 *   - Normal range: Single green LED indicating distance position
 *   - Below min_distance: First LED red (warning indicator)
 *   - Above max_distance: Last LED red (out of range indicator)
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

    /**
     * @brief Start the display logic system
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_DSP_1: Uses LED controller API for all hardware access
     *   - REQ_DSP_2: Obtains all parameters from config_manager
     *   - REQ_DSP_3: Real-time single-LED visualization, no filtering
     *   - REQ_DSP_4: Below minimum distance warning (red LED at position 0)
     *   - REQ_DSP_5: Out of range display (red LED at last position)
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_DSP_ARCH_1: FreeRTOS task, blocks on distance_sensor_get_latest()
     *   - SPEC_DSP_ARCH_2: Configuration integration (cache at startup)
     *   - SPEC_DSP_ALGO_1: Distance-to-LED position mapping
     *   - SPEC_DSP_ALGO_3: Integer arithmetic for embedded performance
     *
     * Obtains configuration from config_manager, initializes hardware,
     * and starts the display task. No separate init/start lifecycle needed.
     *
     * @return ESP_OK on success, ESP_ERR_* on failure
     *
     * @note Single entry point - handles initialization and task startup
     * @note Task runs on core 1 with 4KB stack at priority 3
     * @note Task blocks on distance_sensor_get_latest() until new data arrives
     * @note Configuration obtained from config_manager API
     */
    esp_err_t display_logic_start(void);

#ifdef __cplusplus
}
#endif