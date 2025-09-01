/**
 * @file web_server.c
 * @brief HTTP server for WiFi captive portal and configuration interface
 *
 * This module provides a comprehensive HTTP server for ESP32 WiFi configuration
 * and device management. It serves both a captive portal for WiFi setup and
 * a full web interface for device monitoring and control.
 *
 * Features:
 * - Static file serving from embedded flash assets (HTML, CSS, JS)
 * - WiFi configuration API endpoints (scan, connect, status, reset)
 * - Multi-page responsive web interface with navbar navigation
 * - Integration with DNS server for captive portal functionality
 */

#include "web_server.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include "distance_sensor.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "cJSON.h"
#include <time.h>

static const char *TAG = "web_server";

// Restart timer handle for safe device restart after configuration save
static esp_timer_handle_t restart_timer = NULL;

// Timer callback for delayed restart
static void restart_timer_callback(void* arg)
{
    ESP_LOGI(TAG, "Restarting device now...");
    esp_restart();
}

// Server handles
static httpd_handle_t server = NULL;

// Configuration
static web_server_config_t current_config;
static bool server_running = false;

// Forward declarations for HTTP request handlers
static esp_err_t root_handler(httpd_req_t *req);
static esp_err_t config_handler(httpd_req_t *req);
static esp_err_t scan_handler(httpd_req_t *req);
static esp_err_t connect_handler(httpd_req_t *req);
static esp_err_t status_handler(httpd_req_t *req);
static esp_err_t reset_handler(httpd_req_t *req);

// Configuration management handlers (REQ-CFG-7)
static esp_err_t config_get_handler(httpd_req_t *req);
static esp_err_t config_set_handler(httpd_req_t *req);
static esp_err_t config_reset_handler(httpd_req_t *req);

// System health and diagnostics (REQ-CFG-11)
static esp_err_t system_health_handler(httpd_req_t *req);

// Distance sensor data endpoint
static esp_err_t distance_data_handler(httpd_req_t *req);

// CORS support for API endpoints
static esp_err_t cors_preflight_handler(httpd_req_t *req);

// Static file serving declarations
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t wifi_setup_html_start[] asm("_binary_wifi_setup_html_start");
extern const uint8_t wifi_setup_html_end[] asm("_binary_wifi_setup_html_end");
extern const uint8_t settings_html_start[] asm("_binary_settings_html_start");
extern const uint8_t settings_html_end[] asm("_binary_settings_html_end");
extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[] asm("_binary_style_css_end");
extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");

// Helper functions for static file serving
static const char *get_mime_type(const char *filename);
static esp_err_t get_embedded_file(const char *filename, const uint8_t **data, size_t *size);

// HTTP request handlers
static esp_err_t root_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Root request - serving main dashboard");

    // Modify the request URI to serve index.html
    char original_uri[64];
    strncpy(original_uri, req->uri, sizeof(original_uri) - 1);
    strcpy((char *)req->uri, "/index.html");

    esp_err_t result = static_file_handler(req);

    // Restore original URI
    strcpy((char *)req->uri, original_uri);

    return result;
}

static esp_err_t config_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Config request - serving WiFi setup page");

    // Modify the request URI to serve the WiFi setup page
    char original_uri[64];
    strncpy(original_uri, req->uri, sizeof(original_uri) - 1);
    strcpy((char *)req->uri, "/wifi-setup.html");

    esp_err_t result = static_file_handler(req);

    // Restore original URI
    strcpy((char *)req->uri, original_uri);

    return result;
}

static esp_err_t scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WiFi scan request");

    httpd_resp_set_type(req, "application/json");
    // CORS: Commented out broad origin for security. Can be re-enabled with specific URL for hybrid GitHub Pages approach
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "https://yourusername.github.io");  // For future hybrid approach

    // Start WiFi scan
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300,
    };

    esp_err_t ret = esp_wifi_scan_start(&scan_config, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi scan failed: %s", esp_err_to_name(ret));

        // If we're in AP mode, we might need to switch to APSTA mode for scanning
        wifi_mode_t current_mode;
        if (esp_wifi_get_mode(&current_mode) == ESP_OK && current_mode == WIFI_MODE_AP)
        {
            ESP_LOGW(TAG, "Switching to APSTA mode for WiFi scanning");
            esp_wifi_set_mode(WIFI_MODE_APSTA);
            vTaskDelay(pdMS_TO_TICKS(100)); // Give time for mode switch
            ret = esp_wifi_scan_start(&scan_config, true);
        }

        if (ret != ESP_OK)
        {
            return httpd_resp_send(req, "{\"error\":\"Scan failed\"}", HTTPD_RESP_USE_STRLEN);
        }
    }

    // Get scan results
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    if (ap_count == 0)
    {
        return httpd_resp_send(req, "{\"networks\":[]}", HTTPD_RESP_USE_STRLEN);
    }

    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (ap_records == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for scan results");
        return httpd_resp_send(req, "{\"error\":\"Memory allocation failed\"}", HTTPD_RESP_USE_STRLEN);
    }

    esp_wifi_scan_get_ap_records(&ap_count, ap_records);

    // Build JSON response
    cJSON *root = cJSON_CreateObject();
    cJSON *networks = cJSON_CreateArray();

    for (int i = 0; i < ap_count; i++)
    {
        cJSON *network = cJSON_CreateObject();
        cJSON_AddStringToObject(network, "ssid", (char *)ap_records[i].ssid);
        cJSON_AddNumberToObject(network, "rssi", ap_records[i].rssi);
        cJSON_AddNumberToObject(network, "authmode", ap_records[i].authmode);
        cJSON_AddItemToArray(networks, network);
    }

    cJSON_AddItemToObject(root, "networks", networks);

    char *json_string = cJSON_Print(root);
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    free(json_string);
    cJSON_Delete(root);
    free(ap_records);

    return ESP_OK;
}

static esp_err_t connect_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WiFi connect request");

    httpd_resp_set_type(req, "application/json");
    // CORS: Commented out broad origin for security. Can be re-enabled with specific URL for hybrid GitHub Pages approach
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "https://yourusername.github.io");  // For future hybrid approach

    // Read POST data
    char *buf = malloc(req->content_len + 1);
    if (buf == NULL)
    {
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Memory allocation failed\"}", HTTPD_RESP_USE_STRLEN);
    }

    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0)
    {
        free(buf);
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Failed to read request\"}", HTTPD_RESP_USE_STRLEN);
    }
    buf[ret] = '\0';

    // Parse JSON
    cJSON *root = cJSON_Parse(buf);
    free(buf);

    if (root == NULL)
    {
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Invalid JSON\"}", HTTPD_RESP_USE_STRLEN);
    }

    cJSON *ssid_json = cJSON_GetObjectItem(root, "ssid");
    cJSON *password_json = cJSON_GetObjectItem(root, "password");

    if (!cJSON_IsString(ssid_json))
    {
        cJSON_Delete(root);
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"SSID required\"}", HTTPD_RESP_USE_STRLEN);
    }

    wifi_credentials_t credentials = {0};
    strncpy(credentials.ssid, ssid_json->valuestring, sizeof(credentials.ssid) - 1);

    if (cJSON_IsString(password_json))
    {
        strncpy(credentials.password, password_json->valuestring, sizeof(credentials.password) - 1);
    }

    cJSON_Delete(root);

    ESP_LOGI(TAG, "Attempting to connect to SSID: %s", credentials.ssid);
    ESP_LOGI(TAG, "Password: '%s' (length: %d)", credentials.password, strlen(credentials.password));

    // Attempt to set credentials and connect
    esp_err_t wifi_ret = wifi_manager_set_credentials(&credentials);
    if (wifi_ret == ESP_OK)
    {
        return httpd_resp_send(req, "{\"success\":true}", HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Connection failed\"}", HTTPD_RESP_USE_STRLEN);
    }
}

static esp_err_t status_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    // CORS: Commented out broad origin for security. Can be re-enabled with specific URL for hybrid GitHub Pages approach
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "https://yourusername.github.io");  // For future hybrid approach

    wifi_status_t status;
    if (wifi_manager_get_status(&status) != ESP_OK)
    {
        return httpd_resp_send(req, "{\"error\":\"Failed to get status\"}", HTTPD_RESP_USE_STRLEN);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "mode", status.mode);
    cJSON_AddStringToObject(root, "ssid", status.connected_ssid);
    cJSON_AddNumberToObject(root, "rssi", status.rssi);
    cJSON_AddBoolToObject(root, "has_credentials", status.has_credentials);

    char ip_str[16];
    if (wifi_manager_get_ip_address(ip_str, sizeof(ip_str)) == ESP_OK)
    {
        cJSON_AddStringToObject(root, "ip", ip_str);
    }

    char *json_string = cJSON_Print(root);
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    free(json_string);
    cJSON_Delete(root);

    return ESP_OK;
}

static esp_err_t reset_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WiFi reset request");

    httpd_resp_set_type(req, "application/json");
    // CORS: Commented out broad origin for security. Can be re-enabled with specific URL for hybrid GitHub Pages approach
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    // httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "https://yourusername.github.io");  // For future hybrid approach

    // Clear WiFi credentials and restart device for clean AP mode startup
    esp_err_t ret = wifi_manager_clear_credentials();
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "WiFi credentials cleared, device will restart in AP mode");

        // Send success response
        httpd_resp_send(req, "{\"success\":true,\"message\":\"Device will restart in AP mode in 3 seconds\"}", HTTPD_RESP_USE_STRLEN);

        // Schedule restart using a timer to allow HTTP response to complete
        if (restart_timer == NULL) {
            esp_timer_create_args_t timer_args = {
                .callback = restart_timer_callback,
                .name = "restart_timer"
            };
            esp_timer_create(&timer_args, &restart_timer);
        }
        
        // Start the restart timer (3 seconds delay for clean restart)
        esp_timer_start_once(restart_timer, 3000000); // 3 seconds in microseconds

        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to clear WiFi credentials: %s", esp_err_to_name(ret));
        return httpd_resp_send(req, "{\"success\":false,\"error\":\"Failed to clear credentials\"}", HTTPD_RESP_USE_STRLEN);
    }
}

// Static file serving functions
static const char *get_mime_type(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if (ext == NULL)
    {
        return "text/plain";
    }

    if (strcmp(ext, ".html") == 0)
    {
        return "text/html";
    }
    else if (strcmp(ext, ".css") == 0)
    {
        return "text/css";
    }
    else if (strcmp(ext, ".js") == 0)
    {
        return "application/javascript";
    }
    else if (strcmp(ext, ".json") == 0)
    {
        return "application/json";
    }
    else
    {
        return "text/plain";
    }
}

static esp_err_t get_embedded_file(const char *filename, const uint8_t **data, size_t *size)
{
    ESP_LOGI(TAG, "Getting embedded file: %s", filename);

    if (strcmp(filename, "/index.html") == 0 || strcmp(filename, "/") == 0)
    {
        *data = index_html_start;
        *size = index_html_end - index_html_start;
        ESP_LOGI(TAG, "Found index.html, size: %zu", *size);
    }
    else if (strcmp(filename, "/wifi-setup.html") == 0)
    {
        *data = wifi_setup_html_start;
        *size = wifi_setup_html_end - wifi_setup_html_start;
        ESP_LOGI(TAG, "Found wifi-setup.html, size: %zu", *size);
    }
    else if (strcmp(filename, "/settings.html") == 0)
    {
        *data = settings_html_start;
        *size = settings_html_end - settings_html_start;
        ESP_LOGI(TAG, "Found settings.html, size: %zu", *size);
    }
    else if (strcmp(filename, "/css/style.css") == 0)
    {
        *data = style_css_start;
        *size = style_css_end - style_css_start;
        ESP_LOGI(TAG, "Found style.css, size: %zu", *size);
    }
    else if (strcmp(filename, "/js/app.js") == 0)
    {
        *data = app_js_start;
        *size = app_js_end - app_js_start;
        ESP_LOGI(TAG, "Found app.js, size: %zu", *size);
    }
    else
    {
        ESP_LOGW(TAG, "File not found in embedded files: %s", filename);
        return ESP_ERR_NOT_FOUND;
    }
    return ESP_OK;
}

esp_err_t static_file_handler(httpd_req_t *req)
{
    const char *uri = req->uri;
    const uint8_t *data;
    size_t size;

    ESP_LOGI(TAG, "Serving static file: %s", uri);

    esp_err_t ret = get_embedded_file(uri, &data, &size);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "File not found: %s", uri);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Set appropriate content type
    const char *mime_type = get_mime_type(uri);
    httpd_resp_set_type(req, mime_type);

    // Set cache headers for static assets
    if (strstr(uri, ".css") || strstr(uri, ".js"))
    {
        httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=3600");
    }
    else
    {
        httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
        httpd_resp_set_hdr(req, "Pragma", "no-cache");
        httpd_resp_set_hdr(req, "Expires", "0");
    }

    return httpd_resp_send(req, (const char *)data, size);
}

// Public functions
esp_err_t web_server_init(const web_server_config_t *config)
{
    if (server != NULL)
    {
        ESP_LOGW(TAG, "Web server already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (config != NULL)
    {
        current_config = *config;
    }
    else
    {
        web_server_config_t default_config = WEB_SERVER_DEFAULT_CONFIG();
        current_config = default_config;
    }

    ESP_LOGI(TAG, "Initializing web server on port %d", current_config.port);

    httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
    httpd_config.server_port = current_config.port;
    httpd_config.max_open_sockets = current_config.max_open_sockets;
    httpd_config.max_uri_handlers = 32; // Increase to 32 to ensure we have plenty of slots
    httpd_config.lru_purge_enable = true;

    ESP_LOGI(TAG, "HTTP config: port=%d, max_sockets=%d, max_handlers=%d",
             httpd_config.server_port, httpd_config.max_open_sockets, httpd_config.max_uri_handlers);

    if (httpd_start(&server, &httpd_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return ESP_FAIL;
    }

    // Register URI handlers with error checking
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL};
    esp_err_t ret = httpd_register_uri_handler(server, &root_uri);
    ESP_LOGI(TAG, "Registered handler for '/' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t config_uri = {
        .uri = "/config",
        .method = HTTP_GET,
        .handler = config_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &config_uri);
    ESP_LOGI(TAG, "Registered handler for '/config' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t scan_uri = {
        .uri = "/scan",
        .method = HTTP_GET,
        .handler = scan_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &scan_uri);
    ESP_LOGI(TAG, "Registered handler for '/scan' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t connect_uri = {
        .uri = "/connect",
        .method = HTTP_POST,
        .handler = connect_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &connect_uri);
    ESP_LOGI(TAG, "Registered handler for '/connect' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &status_uri);
    ESP_LOGI(TAG, "Registered handler for '/status' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t reset_uri = {
        .uri = "/reset",
        .method = HTTP_POST,
        .handler = reset_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &reset_uri);
    ESP_LOGI(TAG, "Registered handler for '/reset' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    // Register configuration management API handlers (REQ-CFG-7)
    httpd_uri_t config_get_uri = {
        .uri = "/api/config",
        .method = HTTP_GET,
        .handler = config_get_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &config_get_uri);
    ESP_LOGI(TAG, "Registered handler for '/api/config' GET - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t config_set_uri = {
        .uri = "/api/config",
        .method = HTTP_POST,
        .handler = config_set_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &config_set_uri);
    ESP_LOGI(TAG, "Registered handler for '/api/config' POST - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t config_reset_uri = {
        .uri = "/api/config/reset",
        .method = HTTP_POST,
        .handler = config_reset_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &config_reset_uri);
    ESP_LOGI(TAG, "Registered handler for '/api/config/reset' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    // Register system health endpoint (REQ-CFG-11)
    httpd_uri_t system_health_uri = {
        .uri = "/api/system/health",
        .method = HTTP_GET,
        .handler = system_health_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &system_health_uri);
    ESP_LOGI(TAG, "Registered handler for '/api/system/health' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    // Register distance data endpoint
    httpd_uri_t distance_data_uri = {
        .uri = "/api/distance",
        .method = HTTP_GET,
        .handler = distance_data_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &distance_data_uri);
    ESP_LOGI(TAG, "Registered handler for '/api/distance' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    // Register CORS preflight handler for all API endpoints
    httpd_uri_t options_uri = {
        .uri = "/api/*",
        .method = HTTP_OPTIONS,
        .handler = cors_preflight_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &options_uri);
    ESP_LOGI(TAG, "Registered CORS preflight handler - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    // Register static file handlers
    httpd_uri_t index_uri = {
        .uri = "/index.html",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &index_uri);
    ESP_LOGI(TAG, "Registered handler for '/index.html' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t wifi_setup_uri = {
        .uri = "/wifi-setup.html",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &wifi_setup_uri);
    ESP_LOGI(TAG, "Registered handler for '/wifi-setup.html' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t settings_uri = {
        .uri = "/settings.html",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &settings_uri);
    ESP_LOGI(TAG, "Registered handler for '/settings.html' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t css_uri = {
        .uri = "/css/style.css",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &css_uri);
    ESP_LOGI(TAG, "Registered handler for '/css/style.css' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    httpd_uri_t js_uri = {
        .uri = "/js/app.js",
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = NULL};
    ret = httpd_register_uri_handler(server, &js_uri);
    ESP_LOGI(TAG, "Registered handler for '/js/app.js' - %s", ret == ESP_OK ? "OK" : esp_err_to_name(ret));

    ESP_LOGI(TAG, "Web server initialized successfully");
    return ESP_OK;
}

esp_err_t web_server_start(void)
{
    if (server == NULL)
    {
        ESP_LOGE(TAG, "Web server not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (server_running)
    {
        ESP_LOGW(TAG, "Web server already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting web server");

    // Start DNS server for captive portal if we're in AP mode
    wifi_mode_t wifi_mode;
    if (esp_wifi_get_mode(&wifi_mode) == ESP_OK &&
        (wifi_mode == WIFI_MODE_AP || wifi_mode == WIFI_MODE_APSTA))
    {
        ESP_LOGI(TAG, "AP mode detected - captive portal available at device IP");
        // Note: Users can navigate directly to device IP (e.g., 192.168.4.1)
        // DNS server removed for simplicity - manual navigation still works
    }
    else
    {
        ESP_LOGI(TAG, "STA mode - web interface available at assigned IP");
    }

    server_running = true;
    ESP_LOGI(TAG, "Web server started successfully");

    return ESP_OK;
}

esp_err_t web_server_stop(void)
{
    if (!server_running)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping web server");

    // Stop HTTP server
    if (server != NULL)
    {
        httpd_stop(server);
        server = NULL;
    }

    server_running = false;
    ESP_LOGI(TAG, "Web server stopped");

    return ESP_OK;
}

bool web_server_is_running(void)
{
    return server_running;
}

uint16_t web_server_get_port(void)
{
    return current_config.port;
}

// =============================================================================
// CONFIGURATION MANAGEMENT HANDLERS (REQ-CFG-7, REQ-CFG-8, REQ-CFG-9)
// =============================================================================

/**
 * @brief GET /api/config - Retrieve current configuration
 */
static esp_err_t config_get_handler(httpd_req_t *req)
{
    ESP_LOGD(TAG, "Handling GET /api/config");

    // Set CORS headers
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Content-Type", "application/json");

    // Get current configuration
    system_config_t config;
    esp_err_t ret = config_get_current(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current configuration: %s", esp_err_to_name(ret));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get configuration");
        return ESP_FAIL;
    }

    // Create JSON response
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    // Add configuration metadata
    cJSON_AddNumberToObject(json, "config_version", config.config_version);
    cJSON_AddNumberToObject(json, "save_count", config.save_count);

    // Add distance sensor configuration
    cJSON *distance = cJSON_CreateObject();
    // Convert mm back to cm for web interface display (100mm = 10.0cm)
    cJSON_AddNumberToObject(distance, "min_distance_cm", config.distance_min_mm / 10.0);
    cJSON_AddNumberToObject(distance, "max_distance_cm", config.distance_max_mm / 10.0);
    cJSON_AddNumberToObject(distance, "measurement_interval_ms", config.measurement_interval_ms);
    cJSON_AddNumberToObject(distance, "sensor_timeout_ms", config.sensor_timeout_ms);
    // Convert fixed-point temperature back to decimal (200 = 20.0°C)
    cJSON_AddNumberToObject(distance, "temperature_c", config.temperature_c_x10 / 10.0);
    // Convert integer factor back to decimal (300 = 0.3)
    cJSON_AddNumberToObject(distance, "smoothing_alpha", config.smoothing_factor / 1000.0);
    cJSON_AddItemToObject(json, "distance_sensor", distance);

    // Add LED configuration
    cJSON *led = cJSON_CreateObject();
    cJSON_AddNumberToObject(led, "count", config.led_count);
    cJSON_AddNumberToObject(led, "brightness", config.led_brightness);
    cJSON_AddItemToObject(json, "led", led);

    // Add WiFi configuration (exclude password for security)
    cJSON *wifi = cJSON_CreateObject();
    cJSON_AddStringToObject(wifi, "ssid", config.wifi_ssid);
    cJSON_AddStringToObject(wifi, "password", ""); // Never expose password
    cJSON_AddNumberToObject(wifi, "ap_channel", config.wifi_ap_channel);
    cJSON_AddNumberToObject(wifi, "ap_max_conn", config.wifi_ap_max_conn);
    cJSON_AddNumberToObject(wifi, "sta_max_retry", config.wifi_sta_max_retry);
    cJSON_AddNumberToObject(wifi, "sta_timeout_ms", config.wifi_sta_timeout_ms);
    cJSON_AddItemToObject(json, "wifi", wifi);

    // Convert to string and send
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "Failed to print JSON");
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON serialization failed");
        return ESP_FAIL;
    }

    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    // Cleanup
    free(json_string);
    cJSON_Delete(json);

    ESP_LOGD(TAG, "Configuration sent successfully");
    return ESP_OK;
}

/**
 * @brief POST /api/config - Update and save configuration
 */
static esp_err_t config_set_handler(httpd_req_t *req)
{
    ESP_LOGD(TAG, "Handling POST /api/config");

    // Set CORS headers
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Content-Type", "application/json");

    // Read request body
    char content[1024];
    int ret = httpd_req_recv(req, content, sizeof(content) - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        } else {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read request body");
        }
        return ESP_FAIL;
    }
    content[ret] = '\0';

    ESP_LOGD(TAG, "Received configuration JSON: %s", content);

    // Parse JSON
    cJSON *json = cJSON_Parse(content);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON format");
        return ESP_FAIL;
    }

    // Get current configuration as base
    system_config_t new_config;
    esp_err_t config_ret = config_get_current(&new_config);
    if (config_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current configuration");
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get current configuration");
        return ESP_FAIL;
    }

    // Update configuration from JSON
    cJSON *distance = cJSON_GetObjectItem(json, "distance_sensor");
    if (distance != NULL) {
        cJSON *item;
        if ((item = cJSON_GetObjectItem(distance, "min_distance_cm")) != NULL && cJSON_IsNumber(item)) {
            // Convert cm to mm (10.0cm = 100mm)
            new_config.distance_min_mm = (uint16_t)(cJSON_GetNumberValue(item) * 10);
        }
        if ((item = cJSON_GetObjectItem(distance, "max_distance_cm")) != NULL && cJSON_IsNumber(item)) {
            // Convert cm to mm (50.0cm = 500mm)
            new_config.distance_max_mm = (uint16_t)(cJSON_GetNumberValue(item) * 10);
        }
        if ((item = cJSON_GetObjectItem(distance, "measurement_interval_ms")) != NULL && cJSON_IsNumber(item)) {
            new_config.measurement_interval_ms = (uint16_t)cJSON_GetNumberValue(item);
        }
        if ((item = cJSON_GetObjectItem(distance, "sensor_timeout_ms")) != NULL && cJSON_IsNumber(item)) {
            new_config.sensor_timeout_ms = (uint32_t)cJSON_GetNumberValue(item);
        }
        if ((item = cJSON_GetObjectItem(distance, "temperature_c")) != NULL && cJSON_IsNumber(item)) {
            // Convert to fixed-point tenths (20.0°C = 200)
            new_config.temperature_c_x10 = (int16_t)(cJSON_GetNumberValue(item) * 10);
        }
        if ((item = cJSON_GetObjectItem(distance, "smoothing_alpha")) != NULL && cJSON_IsNumber(item)) {
            // Convert to integer factor (0.3 = 300)
            new_config.smoothing_factor = (uint16_t)(cJSON_GetNumberValue(item) * 1000);
        }
    }

    cJSON *led = cJSON_GetObjectItem(json, "led");
    if (led != NULL) {
        cJSON *item;
        if ((item = cJSON_GetObjectItem(led, "count")) != NULL && cJSON_IsNumber(item)) {
            new_config.led_count = (uint8_t)cJSON_GetNumberValue(item);
        }
        if ((item = cJSON_GetObjectItem(led, "brightness")) != NULL && cJSON_IsNumber(item)) {
            new_config.led_brightness = (uint8_t)cJSON_GetNumberValue(item);
        }
    }

    cJSON *wifi = cJSON_GetObjectItem(json, "wifi");
    if (wifi != NULL) {
        cJSON *item;
        if ((item = cJSON_GetObjectItem(wifi, "ssid")) != NULL && cJSON_IsString(item)) {
            strncpy(new_config.wifi_ssid, cJSON_GetStringValue(item), CONFIG_WIFI_SSID_MAX_LEN - 1);
            new_config.wifi_ssid[CONFIG_WIFI_SSID_MAX_LEN - 1] = '\0';
        }
        if ((item = cJSON_GetObjectItem(wifi, "password")) != NULL && cJSON_IsString(item)) {
            const char *password = cJSON_GetStringValue(item);
            if (strlen(password) > 0) { // Only update if password is provided
                strncpy(new_config.wifi_password, password, CONFIG_WIFI_PASSWORD_MAX_LEN - 1);
                new_config.wifi_password[CONFIG_WIFI_PASSWORD_MAX_LEN - 1] = '\0';
            }
        }
        if ((item = cJSON_GetObjectItem(wifi, "ap_channel")) != NULL && cJSON_IsNumber(item)) {
            new_config.wifi_ap_channel = (uint8_t)cJSON_GetNumberValue(item);
        }
        if ((item = cJSON_GetObjectItem(wifi, "ap_max_conn")) != NULL && cJSON_IsNumber(item)) {
            new_config.wifi_ap_max_conn = (uint8_t)cJSON_GetNumberValue(item);
        }
        if ((item = cJSON_GetObjectItem(wifi, "sta_max_retry")) != NULL && cJSON_IsNumber(item)) {
            new_config.wifi_sta_max_retry = (uint8_t)cJSON_GetNumberValue(item);
        }
        if ((item = cJSON_GetObjectItem(wifi, "sta_timeout_ms")) != NULL && cJSON_IsNumber(item)) {
            new_config.wifi_sta_timeout_ms = (uint32_t)cJSON_GetNumberValue(item);
        }
    }

    cJSON_Delete(json);

    // Validate and save configuration
    config_ret = config_save(&new_config);
    if (config_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save configuration: %s", esp_err_to_name(config_ret));
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Configuration validation failed");
        return ESP_FAIL;
    }

    // Send success response
    const char *response = "{\"status\":\"success\",\"message\":\"Configuration saved successfully. Device will restart in 3 seconds.\"}";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "Configuration updated and saved successfully. Scheduling device restart...");
    
    // Schedule restart using a timer to avoid blocking the HTTP response
    if (restart_timer == NULL) {
        esp_timer_create_args_t timer_args = {
            .callback = restart_timer_callback,
            .name = "restart_timer"
        };
        esp_timer_create(&timer_args, &restart_timer);
    }
    
    // Start the restart timer (3 seconds delay)
    esp_timer_start_once(restart_timer, 3000000); // 3 seconds in microseconds
    
    return ESP_OK;
}

/**
 * @brief POST /api/config/reset - Reset to factory defaults (REQ-CFG-5)
 */
static esp_err_t config_reset_handler(httpd_req_t *req)
{
    ESP_LOGD(TAG, "Handling POST /api/config/reset");

    // Set CORS headers
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Content-Type", "application/json");

    // Perform factory reset
    esp_err_t ret = config_factory_reset();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Factory reset failed: %s", esp_err_to_name(ret));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Factory reset failed");
        return ESP_FAIL;
    }

    // Send success response
    const char *response = "{\"status\":\"success\",\"message\":\"Configuration reset to factory defaults\"}";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "Configuration reset to factory defaults");
    return ESP_OK;
}




/**
 * @brief GET /api/system/health - System health and diagnostics (REQ-CFG-11)
 */
static esp_err_t system_health_handler(httpd_req_t *req)
{
    ESP_LOGD(TAG, "Handling GET /api/system/health");

    // Set CORS headers
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Content-Type", "application/json");

    // Create JSON response
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    // System uptime
    int64_t uptime_us = esp_timer_get_time();
    cJSON_AddNumberToObject(json, "uptime_seconds", (double)uptime_us / 1000000.0);

    // Memory information
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    cJSON_AddNumberToObject(json, "free_heap_bytes", free_heap);
    cJSON_AddNumberToObject(json, "minimum_free_heap_bytes", min_free_heap);
    cJSON_AddNumberToObject(json, "heap_fragmentation_percent", 
                           ((float)(free_heap - min_free_heap) / free_heap) * 100.0);

    // NVS health check
    size_t nvs_free_entries, nvs_total_entries;
    esp_err_t nvs_health = config_nvs_health_check(&nvs_free_entries, &nvs_total_entries);
    
    cJSON *nvs_info = cJSON_CreateObject();
    cJSON_AddStringToObject(nvs_info, "status", 
                           (nvs_health == ESP_OK) ? "healthy" : 
                           (nvs_health == ESP_ERR_INVALID_STATE) ? "corrupted" : "error");
    cJSON_AddStringToObject(nvs_info, "status_message", esp_err_to_name(nvs_health));
    cJSON_AddNumberToObject(nvs_info, "free_entries", nvs_free_entries);
    cJSON_AddNumberToObject(nvs_info, "total_entries", nvs_total_entries);
    cJSON_AddNumberToObject(nvs_info, "used_entries", nvs_total_entries - nvs_free_entries);
    cJSON_AddItemToObject(json, "nvs", nvs_info);

    // Configuration status
    system_config_t current_config;
    esp_err_t config_status = config_get_current(&current_config);
    cJSON *config_info = cJSON_CreateObject();
    cJSON_AddStringToObject(config_info, "status", 
                           (config_status == ESP_OK) ? "healthy" : "error");
    if (config_status == ESP_OK) {
        cJSON_AddNumberToObject(config_info, "version", current_config.config_version);
        cJSON_AddNumberToObject(config_info, "save_count", current_config.save_count);
    }
    cJSON_AddItemToObject(json, "configuration", config_info);

    // WiFi status (basic info)
    wifi_ap_record_t ap_info;
    esp_err_t wifi_status = esp_wifi_sta_get_ap_info(&ap_info);
    cJSON *wifi_info = cJSON_CreateObject();
    if (wifi_status == ESP_OK) {
        cJSON_AddStringToObject(wifi_info, "status", "connected");
        cJSON_AddStringToObject(wifi_info, "ssid", (char*)ap_info.ssid);
        cJSON_AddNumberToObject(wifi_info, "rssi", ap_info.rssi);
    } else {
        cJSON_AddStringToObject(wifi_info, "status", "disconnected");
    }
    cJSON_AddItemToObject(json, "wifi", wifi_info);

    // Overall system health assessment
    bool system_healthy = (nvs_health == ESP_OK) && 
                         (config_status == ESP_OK) && 
                         (free_heap > 50000); // At least 50KB free

    cJSON_AddStringToObject(json, "overall_status", system_healthy ? "healthy" : "degraded");
    cJSON_AddStringToObject(json, "device_type", "ESP32 Distance Sensor");
    cJSON_AddStringToObject(json, "firmware_version", "1.0.0");

    // Convert to string and send
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "Failed to print JSON");
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON serialization failed");
        return ESP_FAIL;
    }

    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    // Cleanup
    free(json_string);
    cJSON_Delete(json);

    ESP_LOGD(TAG, "System health information sent successfully");
    return ESP_OK;
}

/**
 * @brief GET /api/distance - Get current distance measurement
 */
static esp_err_t distance_data_handler(httpd_req_t *req)
{
    ESP_LOGD(TAG, "Distance data requested");

    // Set JSON content type
    httpd_resp_set_type(req, "application/json");
    
    // Set CORS headers
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    // Get latest distance measurement
    distance_measurement_t measurement;
    esp_err_t ret = distance_sensor_get_latest(&measurement);
    
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get distance measurement: %s", esp_err_to_name(ret));
        return httpd_resp_send(req, "{\"error\":\"Failed to get sensor data\"}", HTTPD_RESP_USE_STRLEN);
    }

    // Create JSON response
    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        return httpd_resp_send_500(req);
    }

    // Add measurement data - convert mm to cm for web interface display
    cJSON_AddNumberToObject(json, "distance_cm", measurement.distance_mm / 10.0);
    cJSON_AddNumberToObject(json, "timestamp_us", (double)measurement.timestamp_us);
    
    // Add status as string for better readability
    const char* status_str;
    switch (measurement.status) {
        case DISTANCE_SENSOR_OK:
            status_str = "ok";
            break;
        case DISTANCE_SENSOR_TIMEOUT:
            status_str = "timeout";
            break;
        case DISTANCE_SENSOR_OUT_OF_RANGE:
            status_str = "out_of_range";
            break;
        case DISTANCE_SENSOR_NO_ECHO:
            status_str = "no_echo";
            break;
        case DISTANCE_SENSOR_INVALID_READING:
            status_str = "invalid";
            break;
        default:
            status_str = "unknown";
            break;
    }
    cJSON_AddStringToObject(json, "status", status_str);
    
    // Add timestamp as ISO string for convenience
    time_t timestamp_sec = measurement.timestamp_us / 1000000;
    struct tm timeinfo;
    gmtime_r(&timestamp_sec, &timeinfo);
    char iso_time[32];
    strftime(iso_time, sizeof(iso_time), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    cJSON_AddStringToObject(json, "timestamp_iso", iso_time);

    // Convert to string and send
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "Failed to convert JSON to string");
        cJSON_Delete(json);
        return httpd_resp_send_500(req);
    }

    esp_err_t send_ret = httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);
    
    // Cleanup
    free(json_string);
    cJSON_Delete(json);
    
    return send_ret;
}

/**
 * @brief OPTIONS /api/ wildcard - CORS preflight handler
 */
static esp_err_t cors_preflight_handler(httpd_req_t *req)
{
    ESP_LOGD(TAG, "Handling CORS preflight request");

    // Set CORS headers
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400"); // 24 hours

    // Send empty response
    httpd_resp_send(req, "", 0);
    return ESP_OK;
}