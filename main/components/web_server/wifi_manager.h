
/**
 * @file wifi_manager.h
 * @brief Smart WiFi management with captive portal and credential persistence
 *
 * @author ESP32 Template Project
 * @date 2025-11-11
 *
 * @section architecture_notes Architecture Notes
 * - Manages WiFi connection state machine (AP/STA/Disconnected)
 * - Provides captive portal for WiFi setup (AP mode)
 * - Stores credentials in NVS and handles fallback logic
 * - Integrates with web_server and config_manager components
 * - Designed for modular, component-based ESP-IDF architecture
 *
 * @section requirement_traceability Requirement/Design Traceability
 * - REQ_SYS_NET_1: WiFi connectivity
 * - REQ_SYS_WEB_1: Web-based configuration
 * - REQ_WEB_3: WiFi setup interface
 * - REQ_WEB_5: HTTP server concurrency (indirect)
 * - REQ_SYS_CFG_1: Non-volatile config storage
 * - REQ_SYS_ARCH_1: Component-based architecture
 * - SPEC_WEB_SERVER: Web server design
 *
 * @note See docs/11_requirements/req_web_server.rst and req_system.rst for details.
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

esp_err_t wifi_manager_init(void);

/**
 * @brief Initialize WiFi manager with smart boot logic
 *
 * Automatically tries stored credentials, falls back to AP mode if needed.
 *
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_SYS_NET_1, REQ_WEB_3
 * @design SPEC_WEB_SERVER
 */

esp_err_t wifi_manager_start(void);

/**
 * @brief Start WiFi manager (called after init)
 *
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_SYS_NET_1, REQ_WEB_3
 */

esp_err_t wifi_manager_stop(void);

/**
 * @brief Stop WiFi manager
 *
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_SYS_NET_1
 */

esp_err_t wifi_manager_get_status(wifi_status_t *status);

/**
 * @brief Get current WiFi status
 *
 * @param status Pointer to status structure to fill
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if status is NULL
 *
 * @requirement REQ_SYS_NET_1, REQ_WEB_3
 */

esp_err_t wifi_manager_set_credentials(const wifi_credentials_t *credentials);

/**
 * @brief Save WiFi credentials and attempt connection
 *
 * Stores credentials in NVS and attempts to connect in STA mode.
 * Falls back to AP mode if connection fails after retries.
 *
 * @param credentials WiFi credentials to save and connect to
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_WEB_3, REQ_SYS_CFG_1
 */

esp_err_t wifi_manager_clear_credentials(void);

/**
 * @brief Clear stored WiFi credentials
 *
 * Removes credentials from NVS and switches to AP mode.
 *
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_WEB_3, REQ_SYS_CFG_1
 */

esp_err_t wifi_manager_get_ip_address(char *ip_str, size_t max_len);

/**
 * @brief Get current IP address as string
 *
 * @param ip_str Buffer to store IP address string (min 16 bytes)
 * @param max_len Maximum length of ip_str buffer
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if no IP assigned
 *
 * @requirement REQ_SYS_NET_1
 */

esp_err_t wifi_manager_switch_to_ap(void);

/**
 * @brief Force switch to AP mode
 *
 * Used for manual fallback or reset scenarios.
 *
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_WEB_3, REQ_SYS_NET_1
 */

esp_err_t wifi_manager_monitor(void);

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
 *
 * @requirement REQ_SYS_NET_1
 */

#ifdef __cplusplus
}
#endif