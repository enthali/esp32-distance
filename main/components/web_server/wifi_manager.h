/**
 * @file wifi_manager.h
 * @brief Smart WiFi management with captive portal and credential persistence
 *
 * Implements intelligent WiFi connection logic:
 * - Boot: Try stored credentials → fallback to AP mode if no config/connection fails
 * - AP Mode: Create "ESP32-Distance-Sensor" network with captive portal
 * - STA Mode: Connect to home WiFi with retry logic (3 attempts, 5s timeout)
 * - Persistence: Store WiFi credentials in NVS flash
 */

#pragma once

#include "esp_err.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi manager modes
 */
typedef enum {
    WIFI_MODE_DISCONNECTED = 0,  ///< Not connected to any network
    WIFI_MODE_STA_CONNECTING,    ///< Attempting to connect to STA
    WIFI_MODE_STA_CONNECTED,     ///< Connected to STA network
    WIFI_MODE_AP_ACTIVE,         ///< AP mode active with captive portal
    WIFI_MODE_SWITCHING          ///< Transitioning between modes
} wifi_manager_mode_t;

/**
 * @brief WiFi credentials structure
 */
typedef struct {
    char ssid[32];         ///< WiFi SSID (max 31 chars + null terminator)
    char password[64];     ///< WiFi password (max 63 chars + null terminator)
} wifi_credentials_t;

/**
 * @brief WiFi status information
 */
typedef struct {
    wifi_manager_mode_t mode;       ///< Current WiFi mode
    char connected_ssid[32];        ///< Currently connected SSID (if any)
    int8_t rssi;                   ///< Signal strength (dBm)
    uint8_t retry_count;           ///< Current retry attempt
    bool has_credentials;          ///< Whether stored credentials exist
} wifi_status_t;

/**
 * @brief Initialize WiFi manager with smart boot logic
 * 
 * Automatically tries stored credentials, falls back to AP mode if needed.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Start WiFi manager (called after init)
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_start(void);

/**
 * @brief Stop WiFi manager
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_stop(void);

/**
 * @brief Get current WiFi status
 * 
 * @param status Pointer to status structure to fill
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if status is NULL
 */
esp_err_t wifi_manager_get_status(wifi_status_t *status);

/**
 * @brief Save WiFi credentials and attempt connection
 * 
 * Stores credentials in NVS and attempts to connect in STA mode.
 * Falls back to AP mode if connection fails after retries.
 * 
 * @param credentials WiFi credentials to save and connect to
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_set_credentials(const wifi_credentials_t *credentials);

/**
 * @brief Clear stored WiFi credentials
 * 
 * Removes credentials from NVS and switches to AP mode.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_clear_credentials(void);

/**
 * @brief Get current IP address as string
 * 
 * @param ip_str Buffer to store IP address string (min 16 bytes)
 * @param max_len Maximum length of ip_str buffer
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if no IP assigned
 */
esp_err_t wifi_manager_get_ip_address(char *ip_str, size_t max_len);

/**
 * @brief Force switch to AP mode
 * 
 * Used for manual fallback or reset scenarios.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wifi_manager_switch_to_ap(void);

/**
 * @brief Perform lightweight WiFi health monitoring
 * 
 * This function should be called periodically from main.c (at least once every 30 seconds).
 * Uses internal timing to log WiFi status exactly every 30 seconds regardless of call frequency.
 * 
 * What it monitors:
 * - WiFi connection status and mode
 * - IP address information
 * - SSID and signal strength
 * - Connection health logging
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_manager_monitor(void);

#ifdef __cplusplus
}
#endif