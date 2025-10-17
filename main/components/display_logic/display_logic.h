/**
 * @file display_logic.h
 * @brief LED Distance Visualization Business Logic Component
 *
 * Business logic component that converts distance measurements into LED strip visualizations
 * with interactive animations for garage parking assistance.
 * Provides clean separation between hardware abstraction (components) and application logic.
 *
 * FEATURES:
 * =========
 * - Distance Range Mapping: Configurable range mapped linearly to configured LED count
 * - LED Spacing: Approximately (max_distance - min_distance) / led_count cm per LED
 * - Interactive Animations:
 *   - Ideal Zone (LEDs 10-13): 4 steady RED LEDs (no animation)
 *   - Too Far (LEDs 14-39): Running light GREEN animation toward LED 13
 *   - Too Close (LEDs 0-9): Running light RED animation toward LED 10
 *   - Out of Range (far): Steady GREEN LED at position 39
 *   - Emergency (too close): Blinking pattern on LEDs 0, 10, 20, 30
 *
 * ARCHITECTURE:
 * =============
 * - Priority 3 Task: LED visualization (between sensor priority 6 and test priority 2)
 * - Animation state machine with timing control
 * - Blocking reads from distance sensor queue (waits for new measurements)
 * - Update rate matches distance sensor measurement rate
 * - Animation timing independent of measurement rate
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
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Display mode enumeration
     * 
     * Defines the different animation and display modes for LED strip.
     * 
     * REQ-DSP-IMPL-04: Animation timing and state machine
     */
    typedef enum {
        DISPLAY_MODE_STATIC,           ///< Single LED or fixed pattern (no animation)
        DISPLAY_MODE_RUNNING_FORWARD,  ///< Animation toward ideal zone (too far) - REQ-DSP-ANIM-01
        DISPLAY_MODE_RUNNING_BACKWARD, ///< Animation away from ideal zone (too close) - REQ-DSP-ANIM-02
        DISPLAY_MODE_BLINK_PATTERN,    ///< Blinking every 10th LED (emergency) - REQ-DSP-ANIM-05
        DISPLAY_MODE_IDEAL_STEADY      ///< 4 red LEDs steady (ideal zone) - REQ-DSP-ANIM-03
    } display_mode_t;

    /**
     * @brief Display state structure
     * 
     * Maintains animation state for smooth transitions and timing control.
     * 
     * REQ-DSP-IMPL-04: Animation timing and state machine
     */
    typedef struct {
        display_mode_t mode;             ///< Current display mode
        uint8_t current_position;        ///< Current animation LED position
        uint8_t target_position;         ///< Target LED position for animation
        uint8_t measurement_position;    ///< LED position from distance measurement
        TickType_t last_update;          ///< Last animation update time
        bool blink_state;                ///< Blink on/off state
        uint32_t step_delay_ms;          ///< Dynamic step delay for constant animation duration (REQ-DSP-ANIM-01 AC-1a)
        uint8_t ideal_zone_start;        ///< Dynamic ideal zone start boundary (REQ-DSP-ANIM-03 AC-5)
        uint8_t ideal_zone_end;          ///< Dynamic ideal zone end boundary (REQ-DSP-ANIM-03 AC-5)
    } display_state_t;

    /**
     * @brief Start the display logic system
     * 
     * Obtains configuration from config_manager, initializes hardware,
     * and starts the display task. No separate init/start lifecycle needed.
     * 
     * @return ESP_OK on success, ESP_ERR_* on failure
     * 
     * @note Single entry point - handles initialization and task startup
     * @note Task runs on core 1 with 4KB stack at priority 3
     * @note Task blocks on distance_sensor_get_latest() until new data arrives
     * @note Configuration obtained from config_manager API (REQ-CFG-2)
     */
    esp_err_t display_logic_start(void);

#ifdef __cplusplus
}
#endif