/**
 * @file web_server.c
 * @brief HTTPS server for WiFi captive portal and configuration interface
 *
 * This module provides a comprehensive HTTPS server for ESP32 WiFi configuration
 * and device management. It serves both a captive portal for WiFi setup and
 * a full web interface for device monitoring and control with SSL/TLS encryption.
 *
 * Features:
 * - Static file serving from embedded flash assets (HTML, CSS, JS)
 * - WiFi configuration API endpoints (scan, connect, status, reset)
 * - Multi-page responsive web interface with navbar navigation
 * - Integration with DNS server for captive portal functionality
 * - SSL/TLS encryption using embedded self-signed certificates
 */

#include "web_server.h"
#include "wifi_manager.h"
#include "dns_server.h"
#include "cert_handler.h"
#include "esp_log.h"
#include "esp_https_server.h"
#include "esp_wifi.h"
#include "cJSON.h"

static const char *TAG = "web_server";

// Server handles
static httpd_handle_t server = NULL;

// Configuration
static web_server_config_t current_config;
static bool server_running = false;

// Forward declarations for HTTPS request handlers
static esp_err_t root_handler(httpd_req_t *req);
static esp_err_t config_handler(httpd_req_t *req);
static esp_err_t scan_handler(httpd_req_t *req);
static esp_err_t connect_handler(httpd_req_t *req);
static esp_err_t status_handler(httpd_req_t *req);
static esp_err_t reset_handler(httpd_req_t *req);

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

// HTTPS request handlers
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

    // Clear WiFi credentials and restart in AP mode
    esp_err_t ret = wifi_manager_clear_credentials();
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "WiFi credentials cleared, device will restart in AP mode");

        // Send success response
        httpd_resp_send(req, "{\"success\":true,\"message\":\"Device will restart in AP mode\"}", HTTPD_RESP_USE_STRLEN);

        // Give time for response to be sent, then restart
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();

        return ESP_OK;
    }
    else
    {
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

    ESP_LOGI(TAG, "Initializing HTTPS server on port %d", current_config.port);

    // Initialize certificate handler
    esp_err_t cert_ret = cert_handler_init();
    if (cert_ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize certificate handler: %s", esp_err_to_name(cert_ret));
        return cert_ret;
    }

    // Get SSL certificates
    const char *server_cert_data, *server_key_data;
    size_t server_cert_len, server_key_len;
    
    cert_ret = cert_handler_get_server_cert(&server_cert_data, &server_cert_len);
    if (cert_ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get server certificate: %s", esp_err_to_name(cert_ret));
        return cert_ret;
    }
    
    cert_ret = cert_handler_get_server_key(&server_key_data, &server_key_len);
    if (cert_ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get server private key: %s", esp_err_to_name(cert_ret));
        return cert_ret;
    }

    // Configure HTTPS server
    httpd_ssl_config_t httpd_ssl_config = HTTPD_SSL_CONFIG_DEFAULT();
    httpd_ssl_config.httpd.server_port = current_config.port;
    httpd_ssl_config.httpd.max_open_sockets = current_config.max_open_sockets;
    httpd_ssl_config.httpd.max_uri_handlers = 32; // Increase to 32 to ensure we have plenty of slots
    httpd_ssl_config.httpd.lru_purge_enable = true;
    
    // SSL certificate configuration
    httpd_ssl_config.servercert = (const uint8_t *)server_cert_data;
    httpd_ssl_config.servercert_len = server_cert_len;
    httpd_ssl_config.prvtkey_pem = (const uint8_t *)server_key_data;
    httpd_ssl_config.prvtkey_len = server_key_len;

    ESP_LOGI(TAG, "HTTPS config: port=%d, max_sockets=%d, max_handlers=%d",
             httpd_ssl_config.httpd.server_port, httpd_ssl_config.httpd.max_open_sockets, httpd_ssl_config.httpd.max_uri_handlers);
    ESP_LOGI(TAG, "SSL config: cert_len=%zu, key_len=%zu", server_cert_len, server_key_len);

    if (httpd_ssl_start(&server, &httpd_ssl_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTPS server");
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

    ESP_LOGI(TAG, "Starting HTTPS server");

    // Start DNS server for captive portal if we're in AP mode
    wifi_mode_t wifi_mode;
    if (esp_wifi_get_mode(&wifi_mode) == ESP_OK &&
        (wifi_mode == WIFI_MODE_AP || wifi_mode == WIFI_MODE_APSTA))
    {
        ESP_LOGI(TAG, "Starting DNS server for captive portal (AP mode)");

        // Configure DNS server to redirect to AP IP
        dns_server_config_t dns_config = {
            .port = 53,
            .ap_ip = 0xC0A80401 // 192.168.4.1 in network byte order
        };

        esp_err_t ret = dns_server_start(&dns_config);
        if (ret != ESP_OK)
        {
            ESP_LOGW(TAG, "Failed to start DNS server, captive portal may not work properly");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Skipping DNS server (STA mode - not needed)");
    }

    server_running = true;
    ESP_LOGI(TAG, "HTTPS server started successfully");

    return ESP_OK;
}

esp_err_t web_server_stop(void)
{
    if (!server_running)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping HTTPS server");

    // Stop DNS server
    dns_server_stop();

    // Stop HTTPS server
    if (server != NULL)
    {
        httpd_ssl_stop(server);
        server = NULL;
    }

    server_running = false;
    ESP_LOGI(TAG, "HTTPS server stopped");

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