
/**
 * @file web_server.h
 * @brief HTTP server for WiFi captive portal and configuration interface
 *
 * @author ESP32 Template Project
 * @date 2025-11-11
 *
 * @section architecture_notes Architecture Notes
 * - Implements HTTP server and DNS server for captive portal (AP mode)
 * - Serves static web assets from flash (HTML, CSS, JS)
 * - Provides REST API endpoints for configuration, WiFi setup, and LED state
 * - Integrates with config_manager and wifi_manager components
 * - Designed for modular, component-based ESP-IDF architecture
 *
 * @section requirement_traceability Requirement/Design Traceability
 * - REQ_SYS_WEB_1: System web-based configuration
 * - REQ_WEB_1: Real-time status display
 * - REQ_WEB_2: Configuration interface
 * - REQ_WEB_3: WiFi setup interface
 * - REQ_WEB_CONF_1: Configuration REST API
 * - REQ_WEB_SCHEMA_1: Schema-driven config form
 * - REQ_WEB_4: Web interface navigation
 * - REQ_WEB_5: HTTP server concurrency
 * - REQ_WEB_LED_1: LED state API endpoint
 * - REQ_SYS_NET_1: WiFi connectivity
 * - REQ_SYS_CFG_1: Non-volatile config storage
 * - REQ_SYS_ARCH_1: Component-based architecture
 * - SPEC_WEB_SERVER: Web server design
 * - SPEC_CONFIG_WEBPAGE: Web UI design
 *
 * @note See docs/11_requirements/req_web_server.rst and req_system.rst for details.
 */

#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Web server configuration structure
 */
typedef struct {
    uint16_t port;              ///< HTTP server port (default: 80)
    size_t max_open_sockets;    ///< Maximum concurrent connections (default: 7)
} web_server_config_t;

/**
 * @brief Default web server configuration
 */
#define WEB_SERVER_DEFAULT_CONFIG() { \
    .port = 80, \
    .max_open_sockets = 7 \
}

esp_err_t web_server_init(const web_server_config_t *config);

/**
 * @brief Initialize web server with embedded static assets
 *
 * Sets up HTTP server with all URI handlers for:
 * - Static file serving (HTML, CSS, JS from embedded flash)
 * - WiFi configuration API endpoints (scan, connect, status, reset)
 * - Responsive multi-page web interface with navbar navigation
 *
 * @param config Server configuration (NULL for default settings)
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_SYS_WEB_1, REQ_WEB_1, REQ_WEB_2, REQ_WEB_3, REQ_WEB_CONF_1, REQ_WEB_4, REQ_WEB_5
 * @design SPEC_WEB_SERVER
 */

esp_err_t web_server_start(void);

/**
 * @brief Start web server and DNS server for captive portal
 *
 * Starts the HTTP server and automatically starts the DNS server if in AP mode
 * for captive portal functionality. The DNS server redirects all queries to
 * the ESP32's IP address to enable automatic captive portal detection.
 *
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_WEB_3, REQ_SYS_NET_1
 * @design SPEC_WEB_SERVER
 */

esp_err_t web_server_stop(void);

/**
 * @brief Stop web server and DNS server
 *
 * Gracefully shuts down both HTTP and DNS servers, freeing all resources.
 * Safe to call multiple times or when servers are not running.
 *
 * @return ESP_OK on success, error code on failure
 *
 * @requirement REQ_WEB_5
 * @design SPEC_WEB_SERVER
 */

bool web_server_is_running(void);

/**
 * @brief Check if web server is currently running
 *
 * @return true if HTTP server is active, false otherwise
 *
 * @requirement REQ_WEB_5
 */

uint16_t web_server_get_port(void);

/**
 * @brief Get the current HTTP server port number
 *
 * @return Server port number (typically 80)
 *
 * @requirement REQ_WEB_5
 */

esp_err_t static_file_handler(httpd_req_t *req);

/**
 * @brief HTTP handler for serving embedded static files
 *
 * Serves HTML, CSS, and JavaScript files that are embedded in flash memory
 * at build time. Automatically sets appropriate MIME types and cache headers.
 * Supports the complete multi-page web interface assets.
 *
 * @param req HTTP request object
 * @return ESP_OK on success, ESP_FAIL on file not found or other errors
 *
 * @requirement REQ_WEB_1, REQ_WEB_2, REQ_WEB_4, REQ_WEB_SCHEMA_1
 * @design SPEC_CONFIG_WEBPAGE
 */

#ifdef __cplusplus
}
#endif