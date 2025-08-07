/**
 * @file config_manager.h
 * @brief Configuration Management API for ESP32 Distance Sensor Project
 * 
 * This module provides runtime configuration management with persistent NVS storage,
 * parameter validation, and thread-safe access. It implements the dynamic configuration
 * system requirements for the ESP32 Distance Sensor project.
 * 
 * FEATURES:
 * - Runtime configuration structure matching compile-time defaults
 * - NVS persistence with power-loss protection
 * - Parameter validation with range checking
 * - Thread-safe access with mutex protection
 * - Automatic fallback to factory defaults
 * - Configuration versioning and change tracking
 * 
 * THREAD SAFETY:
 * All functions are thread-safe and can be called from multiple tasks simultaneously.
 * Internal mutex protection ensures data consistency.
 * 
 * ERROR HANDLING:
 * All functions return esp_err_t codes for proper error handling integration
 * with ESP-IDF error handling patterns.
 * 
 * @author ESP32 Distance Project Team
 * @date 2025
 * @version 1.0
 * 
 * Requirements Traceability:
 * - REQ-CFG-3: Configuration Data Structure
 * - REQ-CFG-4: Non-Volatile Storage (NVS)
 * - REQ-CFG-5: Configuration API
 * - REQ-CFG-6: Parameter Validation
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION DATA STRUCTURE (REQ-CFG-3)
// =============================================================================

/**
 * @brief Current configuration version
 * @note Used for compatibility checking and migration
 */
#define CONFIG_VERSION 1

/**
 * @brief Maximum length for WiFi SSID (including null terminator)
 * @note Based on IEEE 802.11 standard (32 chars + null)
 */
#define CONFIG_WIFI_SSID_MAX_LEN 33

/**
 * @brief Maximum length for WiFi password (including null terminator)
 * @note Based on WPA standard (64 chars + null)
 */
#define CONFIG_WIFI_PASSWORD_MAX_LEN 65

/**
 * @brief Runtime configuration structure
 * 
 * Contains all user-configurable parameters with metadata for versioning
 * and change tracking. Optimized for NVS storage efficiency using appropriate
 * data types aligned with ESP32 memory requirements.
 * 
 * @requirement REQ-CFG-3 AC-1-6
 */
typedef struct {
    // Configuration metadata
    uint32_t config_version;         ///< Configuration version (current: 1)
    uint32_t save_count;             ///< Number of times configuration has been saved
    
    // Distance sensor settings (runtime configurable)
    float distance_min_cm;           ///< Minimum distance for LED mapping (5.0-100.0)
    float distance_max_cm;           ///< Maximum distance for LED mapping (20.0-400.0)
    uint16_t measurement_interval_ms; ///< Measurement interval in ms (50-1000)
    uint32_t sensor_timeout_ms;      ///< Sensor timeout in ms (10-50)
    float temperature_c;             ///< Ambient temperature in Celsius (-20.0-60.0)
    float smoothing_alpha;           ///< EMA smoothing factor (0.1-1.0)
    
    // LED settings (runtime configurable)
    uint8_t led_count;               ///< Number of LEDs in strip (1-60)
    uint8_t led_brightness;          ///< LED brightness level (10-255)
    
    // WiFi settings (runtime configurable)
    char wifi_ssid[CONFIG_WIFI_SSID_MAX_LEN];        ///< WiFi network name
    char wifi_password[CONFIG_WIFI_PASSWORD_MAX_LEN]; ///< WiFi network password
    uint8_t wifi_ap_channel;         ///< WiFi AP channel (1-13)
    uint8_t wifi_ap_max_conn;        ///< Maximum AP connections (1-10)
    uint8_t wifi_sta_max_retry;      ///< STA connection retry attempts (1-10)
    uint32_t wifi_sta_timeout_ms;    ///< STA connection timeout in ms (1000-30000)
} system_config_t;

// =============================================================================
// CONFIGURATION API (REQ-CFG-5)
// =============================================================================

/**
 * @brief Initialize configuration management subsystem
 * 
 * Initializes the configuration manager, creates necessary mutex protection,
 * and loads configuration from NVS. If NVS read fails, automatically performs
 * factory reset and saves default configuration.
 * 
 * Must be called once during system startup before any other config functions.
 * 
 * @return ESP_OK on success
 * @return ESP_ERR_NO_MEM if memory allocation fails
 * @return Other ESP error codes for initialization failures
 * 
 * @requirement REQ-CFG-5 AC-1
 */
esp_err_t config_init(void);

/**
 * @brief Load configuration from NVS
 * 
 * Reads current configuration from NVS storage into provided structure.
 * If NVS read fails or validation fails, automatically calls config_factory_reset()
 * to restore defaults and persist them.
 * 
 * @param[out] config Pointer to configuration structure to populate
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if config is NULL
 * @return ESP_ERR_NOT_FOUND if no configuration exists in NVS (triggers factory reset)
 * @return Other ESP error codes for NVS or validation failures
 * 
 * @requirement REQ-CFG-5 AC-2, AC-3
 */
esp_err_t config_load(system_config_t* config);

/**
 * @brief Save configuration to NVS
 * 
 * Validates configuration parameters using config_validate_range() and saves
 * to NVS if validation passes. Updates save_count and performs atomic write
 * operation for power-loss protection.
 * 
 * @param[in] config Pointer to configuration structure to save
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if config is NULL or validation fails
 * @return Other ESP error codes for NVS write failures
 * 
 * @requirement REQ-CFG-5 AC-4
 */
esp_err_t config_save(const system_config_t* config);

/**
 * @brief Validate configuration parameter ranges
 * 
 * Validates all parameters in configuration structure against defined ranges
 * and inter-parameter relationships. Logs specific error messages for invalid
 * parameters.
 * 
 * @param[in] config Pointer to configuration structure to validate
 * @return ESP_OK if all parameters are valid
 * @return ESP_ERR_INVALID_ARG if config is NULL
 * @return ESP_ERR_INVALID_SIZE if any parameter is out of range
 * 
 * @requirement REQ-CFG-5 AC-5, REQ-CFG-6 AC-1-6
 */
esp_err_t config_validate_range(const system_config_t* config);

/**
 * @brief Reset configuration to factory defaults
 * 
 * Restores compile-time defaults from config.h and persists them to NVS
 * using config_save(). Completes the error recovery sequence atomically.
 * 
 * @return ESP_OK on success
 * @return Other ESP error codes for save operation failures
 * 
 * @requirement REQ-CFG-5 AC-6, AC-9
 */
esp_err_t config_factory_reset(void);

/**
 * @brief Check if parameter value is within valid range
 * @param[in] param_name Name of parameter for logging
 * @param[in] value Value to validate
 * @param[in] min_val Minimum valid value
 * @param[in] max_val Maximum valid value
 * @return true if value is within range, false otherwise
 * 
 * @requirement REQ-CFG-6 AC-1-2
 */
bool config_is_valid_range(const char* param_name, float value, float min_val, float max_val);

/**
 * @brief Get current configuration (thread-safe)
 * 
 * Returns a copy of the current configuration structure with mutex protection.
 * This function is safe to call from multiple tasks simultaneously.
 * 
 * @param[out] config Pointer to configuration structure to populate
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if config is NULL
 * @return ESP_ERR_INVALID_STATE if config manager not initialized
 */
esp_err_t config_get_current(system_config_t* config);

/**
 * @brief Update current configuration (thread-safe)
 * 
 * Updates the current configuration with validation and mutex protection.
 * Does not automatically save to NVS - call config_save() separately if
 * persistence is required.
 * 
 * @param[in] config Pointer to new configuration structure
 * @return ESP_OK on success
 * @return ESP_ERR_INVALID_ARG if config is NULL or validation fails
 * @return ESP_ERR_INVALID_STATE if config manager not initialized
 */
esp_err_t config_set_current(const system_config_t* config);

/**
 * @brief Perform NVS health check and diagnostics (REQ-CFG-11)
 * 
 * Checks NVS partition health, available space, and configuration integrity.
 * Can be used for system monitoring and preventive maintenance.
 * 
 * @param[out] free_entries Number of free NVS entries (optional, can be NULL)
 * @param[out] total_entries Total number of NVS entries (optional, can be NULL)
 * @return ESP_OK if NVS is healthy
 * @return ESP_ERR_NVS_CORRUPT if corruption is detected
 * @return Other ESP error codes for NVS issues
 */
esp_err_t config_nvs_health_check(size_t* free_entries, size_t* total_entries);

// =============================================================================
// PARAMETER VALIDATION CONSTANTS (REQ-CFG-6)
// =============================================================================

// Distance sensor parameter ranges
#define CONFIG_DISTANCE_MIN_CM_MIN          5.0f
#define CONFIG_DISTANCE_MIN_CM_MAX          100.0f
#define CONFIG_DISTANCE_MAX_CM_MIN          20.0f
#define CONFIG_DISTANCE_MAX_CM_MAX          400.0f
#define CONFIG_MEASUREMENT_INTERVAL_MS_MIN  50
#define CONFIG_MEASUREMENT_INTERVAL_MS_MAX  1000
#define CONFIG_SENSOR_TIMEOUT_MS_MIN        10
#define CONFIG_SENSOR_TIMEOUT_MS_MAX        50
#define CONFIG_TEMPERATURE_C_MIN            -20.0f
#define CONFIG_TEMPERATURE_C_MAX            60.0f
#define CONFIG_SMOOTHING_ALPHA_MIN          0.1f
#define CONFIG_SMOOTHING_ALPHA_MAX          1.0f

// LED parameter ranges
#define CONFIG_LED_COUNT_MIN                1
#define CONFIG_LED_COUNT_MAX                60
#define CONFIG_LED_BRIGHTNESS_MIN           10
#define CONFIG_LED_BRIGHTNESS_MAX           255

// WiFi parameter ranges
#define CONFIG_WIFI_AP_CHANNEL_MIN          1
#define CONFIG_WIFI_AP_CHANNEL_MAX          13
#define CONFIG_WIFI_AP_MAX_CONN_MIN         1
#define CONFIG_WIFI_AP_MAX_CONN_MAX         10
#define CONFIG_WIFI_STA_MAX_RETRY_MIN       1
#define CONFIG_WIFI_STA_MAX_RETRY_MAX       10
#define CONFIG_WIFI_STA_TIMEOUT_MS_MIN      1000
#define CONFIG_WIFI_STA_TIMEOUT_MS_MAX      30000

#ifdef __cplusplus
}
#endif