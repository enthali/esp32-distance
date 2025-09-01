/**
 * @file distance_sensor.h
 * @brief HC-SR04 Ultrasonic Distance Sensor Driver
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
     * @brief Distance sensor configuration
     */
    typedef struct
    {
        gpio_num_t trigger_pin;           ///< Trigger pin (default: GPIO14)
        gpio_num_t echo_pin;              ///< Echo pin (default: GPIO15)
        uint32_t measurement_interval_ms; ///< Measurement interval (default: 100ms)
        uint32_t timeout_ms;              ///< Echo timeout (default: 30ms)
        int16_t temperature_c_x10;        ///< Temperature for speed of sound compensation in tenths (default: 200 = 20.0°C)
        uint16_t smoothing_factor;        ///< EMA smoothing factor: 0=heavy smoothing, 1000=no smoothing (default: 300 = 0.3)
    } distance_sensor_config_t;

    /**
     * @brief Initialize the distance sensor
     *
     * @param config Sensor configuration, or NULL for defaults
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t distance_sensor_init(const distance_sensor_config_t *config);

    /**
     * @brief Start distance measurements
     *
     * Creates the measurement task that triggers sensors and processes results.
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t distance_sensor_start(void);

    /**
     * @brief Stop distance measurements
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t distance_sensor_stop(void);

    /**
     * @brief Get the latest distance measurement (blocking)
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
     * @return true if new measurement is available, false otherwise
     */
    bool distance_sensor_has_new_measurement(void);

    /**
     * @brief Get queue overflow statistics
     *
     * @return uint32_t Number of measurements discarded due to queue overflow
     */
    uint32_t distance_sensor_get_queue_overflows(void);

    /**
     * @brief Perform lightweight sensor health monitoring
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
     * @return true if sensor task is running, false otherwise
     */
    bool distance_sensor_is_running(void);

#ifdef __cplusplus
}
#endif
