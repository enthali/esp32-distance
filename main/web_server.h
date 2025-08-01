/**
 * @file web_server.h
 * @brief HTTPS server for WiFi captive portal and configuration interface
 *
 * Provides:
 * - Multi-page responsive web interface with navbar navigation
 * - Static file serving from embedded flash assets (HTML, CSS, JS)
 * - WiFi configuration API endpoints (scan, connect, status, reset)
 * - Integration with DNS server module for captive portal functionality
 * - Secure HTTPS communication with embedded SSL certificates
 */

#pragma once

#include "esp_err.h"
#include "esp_https_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Web server configuration structure
 */
typedef struct {
    uint16_t port;              ///< HTTPS server port (default: 443)
    size_t max_open_sockets;    ///< Maximum concurrent connections (default: 4, ESP32 optimized)
} web_server_config_t;

/**
 * @brief Default web server configuration
 */
#define WEB_SERVER_DEFAULT_CONFIG() { \
    .port = 443, \
    .max_open_sockets = 4 \
}

/**
 * @brief Initialize HTTPS server with embedded static assets and SSL certificates
 * 
 * Sets up HTTPS server with all URI handlers for:
 * - Static file serving (HTML, CSS, JS from embedded flash)
 * - WiFi configuration API endpoints (scan, connect, status, reset)
 * - Responsive multi-page web interface with navbar navigation
 * - SSL/TLS encryption using embedded certificates
 * 
 * @param config Server configuration (NULL for default settings: port 443, 4 max sockets)
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_init(const web_server_config_t *config);

/**
 * @brief Start HTTPS server and DNS server for captive portal
 * 
 * Starts the HTTPS server and automatically starts the DNS server if in AP mode
 * for captive portal functionality. The DNS server redirects all queries to
 * the ESP32's IP address to enable automatic captive portal detection.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_start(void);

/**
 * @brief Stop HTTPS server and DNS server
 * 
 * Gracefully shuts down both HTTPS and DNS servers, freeing all resources.
 * Safe to call multiple times or when servers are not running.
 * 
 * @return ESP_OK on success, error code on failure
 */
esp_err_t web_server_stop(void);

/**
 * @brief Check if HTTPS server is currently running
 * 
 * @return true if HTTPS server is active, false otherwise
 */
bool web_server_is_running(void);

/**
 * @brief Get the current HTTPS server port number
 * 
 * @return Server port number (typically 443)
 */
uint16_t web_server_get_port(void);

/**
 * @brief HTTPS handler for serving embedded static files
 * 
 * Serves HTML, CSS, and JavaScript files that are embedded in flash memory
 * at build time. Automatically sets appropriate MIME types and cache headers.
 * Supports the complete multi-page web interface assets.
 * 
 * @param req HTTPS request object
 * @return ESP_OK on success, ESP_FAIL on file not found or other errors
 */
esp_err_t static_file_handler(httpd_req_t *req);

#ifdef __cplusplus
}
#endif