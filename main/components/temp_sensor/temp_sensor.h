/**
 * @file temp_sensor.h
 * @brief DS18B20 Dallas 1-Wire Temperature Sensor Component
 *
 * REQUIREMENTS TRACEABILITY:
 *   - REQ_TEMP_1: Component initialization
 *   - REQ_TEMP_2: Dallas 1-Wire protocol
 *   - REQ_TEMP_3: Periodic measurement task
 *   - REQ_TEMP_4: Temperature data API
 *   - REQ_TEMP_5: Live feed to distance sensor
 *
 * DESIGN TRACEABILITY:
 *   - SPEC_TEMP_ARCH_1:   Component architecture and data flow
 *   - SPEC_TEMP_ONEWIRE_1: 1-Wire bit-banging protocol
 *   - SPEC_TEMP_TASK_1:   Measurement task design
 *   - SPEC_TEMP_API_1:    Public API and data structures
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Temperature sensor configuration */
typedef struct {
    gpio_num_t data_pin;    ///< 1-Wire data GPIO (default: GPIO4, from config key "temp_gpio")
    uint32_t   interval_ms; ///< Measurement interval ms (default: 2000, from config key "temp_interval_ms")
} temp_sensor_config_t;

/** @brief Single temperature measurement */
typedef struct {
    int16_t  temperature_c_x10; ///< Temperature in 0.1°C units (215 = 21.5°C)
    bool     valid;             ///< true if successfully measured, false on startup or error
    uint64_t timestamp_us;      ///< Measurement time from esp_timer_get_time()
} temp_measurement_t;

/**
 * @brief Initialise the temperature sensor component.
 *
 * Pass NULL to load configuration from config_manager (keys: temp_gpio, temp_interval_ms).
 *
 * REQUIREMENTS TRACEABILITY: REQ_TEMP_1
 *
 * @param config  Pointer to configuration struct, or NULL for config_manager defaults.
 * @return ESP_OK on success.
 */
esp_err_t temp_sensor_init(const temp_sensor_config_t *config);

/**
 * @brief Create and start the measurement task.
 *
 * REQUIREMENTS TRACEABILITY: REQ_TEMP_3
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already running.
 */
esp_err_t temp_sensor_start(void);

/**
 * @brief Stop and delete the measurement task.
 *
 * @return ESP_OK on success.
 */
esp_err_t temp_sensor_stop(void);

/**
 * @brief Get the latest temperature measurement (non-blocking, thread-safe).
 *
 * REQUIREMENTS TRACEABILITY: REQ_TEMP_4
 *
 * @param out  Pointer to store measurement. Must not be NULL.
 * @return ESP_OK on success.
 * @return ESP_ERR_INVALID_STATE if no measurement has been taken yet.
 * @return ESP_ERR_INVALID_ARG if out is NULL.
 */
esp_err_t temp_sensor_get_latest(temp_measurement_t *out);

#ifdef __cplusplus
}
#endif
