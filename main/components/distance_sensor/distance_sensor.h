/**
 * @file distance_sensor.h
 * @brief HC-SR04 Ultrasonic Distance Sensor Driver
 *
 * REQUIREMENTS TRACEABILITY:
 *   - REQ_SNS_1: Component initialization (GPIO, ISR, queues)
 *   - REQ_SNS_2: Trigger pulse generation
 *   - REQ_SNS_3: Real-time timestamp capture (ISR)
 *   - REQ_SNS_4: Measurement processing, filtering, validation
 *   - REQ_SNS_5: Temperature compensation
 *   - REQ_SNS_6: Queue overflow handling
 *   - REQ_SNS_7: Health monitoring
 *   - REQ_SNS_8: API blocking/read semantics
 *   - REQ_SNS_9: Race condition safety
 *   - REQ_SNS_10: Statistics and diagnostics
 *
 * DESIGN TRACEABILITY:
 *   - SPEC_SNS_ARCH_1: Dual-queue real-time architecture
 *   - SPEC_SNS_ALGO_1: Distance calculation and filtering
 *   - SPEC_SNS_API_1: API blocking/read semantics
 *
 * Interrupt-driven driver with dual-queue architecture for real-time performance.
 *
 * ARCHITECTURE OVERVIEW:
 * =====================
 *
 * ISR (echo_isr_handler)
 * ├── Captures timestamps only (IRAM_ATTR for real-time performance)
 * ├── No floating-point calculations (real-time safe)
 * ├── RISING edge: Start timing
 * ├── FALLING edge: Send raw timestamps to queue
 * └── Sends to raw_measurement_queue → Sensor Task
 *
 * Sensor Task (distance_sensor_task)
 * ├── Triggers measurements every 100ms (configurable)
 * ├── Waits on raw_measurement_queue (blocking with timeout)
 * ├── Processes calculations & validation (floating-point safe)
 * ├── Temperature compensation for speed of sound
 * ├── Range validation (2-400cm)
 * ├── Queue overflow handling with statistics
 * ├── Sends to processed_measurement_queue → API
 * └── Sleeps between measurements (CPU friendly, allows other tasks)
 *
 * Public API
 * ├── distance_sensor_get_latest() - Blocking read (waits for new data)
 * ├── distance_sensor_has_new_measurement() - Check availability
 * ├── Queue overflow detection & statistics
 * └── Race condition safe (no shared variables)
 *
 * REAL-TIME DESIGN PRINCIPLES:
 * ============================
 * - ISR only captures timestamps (no calculations)
 * - Dual-queue eliminates race conditions
 * - Task yields CPU between measurements
 * - API blocks until new data is available
 * - Automatic queue overflow handling
 * - Temperature compensation for accuracy
 *
 * USAGE EXAMPLE:
 * =============
 * ```c
 * // Initialize and start
 * distance_sensor_init(NULL);  // Use defaults
 * distance_sensor_start();
 *
 * // Read measurements
 * distance_measurement_t measurement;
 * if (distance_sensor_get_latest(&measurement) == ESP_OK) {
 *     printf("Distance: %d mm\n", measurement.distance_mm);
 * }
 * ```
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Distance sensor error codes
     */
    typedef enum
    {
        DISTANCE_SENSOR_OK = 0,
        DISTANCE_SENSOR_TIMEOUT,
        DISTANCE_SENSOR_OUT_OF_RANGE,
        DISTANCE_SENSOR_NO_ECHO,
        DISTANCE_SENSOR_INVALID_READING
    } distance_sensor_error_t;

    /**
     * @brief Raw measurement data from ISR (timestamps only)
     */
    typedef struct
    {
        uint64_t echo_start_us;         ///< Echo start timestamp (microseconds)
        uint64_t echo_end_us;           ///< Echo end timestamp (microseconds)
        distance_sensor_error_t status; ///< Measurement status
    } distance_raw_measurement_t;

    /**
     * @brief Processed distance measurement
     */
    typedef struct
    {
        uint16_t distance_mm;           ///< Calculated distance in millimeters (0-65535mm = 65.5m range)
        uint64_t timestamp_us;          ///< Measurement timestamp (microseconds)
        distance_sensor_error_t status; ///< Measurement status
    } distance_measurement_t;

    /**
     * @brief Initialize the distance sensor
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_1: Initialization (GPIO, ISR, queues)
     *   - REQ_SNS_5: Temperature compensation
     *   - REQ_SNS_6: Queue overflow handling
     *   - REQ_SNS_9: Race condition safety
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_ARCH_1: Dual-queue architecture
     *
     * Loads configuration from config_manager automatically.
     * Configuration parameters are read from NVS using these keys:
     * - measurement_interval_ms: Time between measurements
     * - sensor_timeout_ms: Maximum time to wait for echo
     * - temperature_c_x10: Temperature compensation (tenths of Celsius)
     * - smoothing_factor: EMA filter alpha value (0-1000)
     *
     * @param trigger_pin GPIO pin for HC-SR04 trigger signal
     * @param echo_pin GPIO pin for HC-SR04 echo signal
     * @return esp_err_t ESP_OK on success
     * @return ESP_ERR_INVALID_STATE if config_manager not initialized
     * @return ESP_ERR_INVALID_ARG if pins are invalid
     * @return ESP_ERR_* for other configuration or hardware errors
     */
    esp_err_t distance_sensor_init(gpio_num_t trigger_pin, gpio_num_t echo_pin);

    /**
     * @brief Start distance measurements
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_2: Trigger pulse generation
     *   - REQ_SNS_3: ISR timestamp capture
     *   - REQ_SNS_4: Measurement processing
     *   - REQ_SNS_8: API blocking/read semantics
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_ARCH_1: Dual-queue architecture
     *
     * Creates the measurement task that triggers sensors and processes results.
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t distance_sensor_start(void);

    /**
     * @brief Stop distance measurements
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_8: API blocking/read semantics
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_API_1: API blocking/read semantics
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t distance_sensor_stop(void);

    /**
     * @brief Get the latest distance measurement (blocking)
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_8: API blocking/read semantics
     *   - REQ_SNS_9: Race condition safety
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_API_1: API blocking/read semantics
     *
     * Waits on the internal queue until a new measurement is available.
     * This function will block until new data arrives from the sensor task.
     *
     * @param measurement Pointer to store the measurement result
     * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG if measurement is NULL
     */
    esp_err_t distance_sensor_get_latest(distance_measurement_t *measurement);

    /**
     * @brief Check if new measurement is available
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_8: API blocking/read semantics
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_API_1: API blocking/read semantics
     *
     * @return true if new measurement is available, false otherwise
     */
    bool distance_sensor_has_new_measurement(void);

    /**
     * @brief Get queue overflow statistics
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_6: Queue overflow handling
     *   - REQ_SNS_10: Statistics and diagnostics
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_ARCH_1: Dual-queue architecture
     *
     * @return uint32_t Number of measurements discarded due to queue overflow
     */
    uint32_t distance_sensor_get_queue_overflows(void);

    /**
     * @brief Perform lightweight sensor health monitoring
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_7: Health monitoring
     *   - REQ_SNS_10: Statistics and diagnostics
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_ARCH_1: Dual-queue architecture
     *
     * This function should be called periodically (every 5-10 seconds) from main.c.
     * It performs the same monitoring that was previously done in main.c but
     * encapsulated within the sensor component.
     *
     * What it monitors:
     * - Queue overflow detection and logging
     * - Component health status
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t distance_sensor_monitor(void);

    /**
     * @brief Check if sensor is running
     *
     * REQUIREMENTS TRACEABILITY:
     *   - REQ_SNS_8: API blocking/read semantics
     *
     * DESIGN TRACEABILITY:
     *   - SPEC_SNS_API_1: API blocking/read semantics
     *
     * @return true if sensor task is running, false otherwise
     */
    bool distance_sensor_is_running(void);

#ifdef __cplusplus
}
#endif
