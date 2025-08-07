/**
 * @file wifi_manager.c
 * @brief Smart WiFi management with captive portal and credential persistence
 */

#include "wifi_manager.h"
#include "web_server.h"
#include "config.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "string.h"
#include "esp_netif_ip_addr.h"

static const char *TAG = "wifi_manager";

// NVS storage keys
#define NVS_NAMESPACE "wifi_config"
#define NVS_SSID_KEY "ssid"
#define NVS_PASSWORD_KEY "password"

// Event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

// State management
static bool wifi_manager_initialized = false;
static wifi_manager_mode_t current_mode = WIFI_MODE_DISCONNECTED;
static wifi_credentials_t stored_credentials = {0};
static bool has_stored_credentials = false;
static uint8_t retry_count = 0;

// Connection timeout timer
static esp_timer_handle_t connection_timeout_timer = NULL;

// ESP-IDF objects
static esp_netif_t *wifi_netif_sta = NULL;
static esp_netif_t *wifi_netif_ap = NULL;
static EventGroupHandle_t wifi_event_group = NULL;

// Forward declarations
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void connection_timeout_callback(void* arg);
static esp_err_t load_credentials_from_nvs(void);
static esp_err_t save_credentials_to_nvs(const wifi_credentials_t *credentials);
static esp_err_t start_sta_mode(void);
static esp_err_t start_ap_mode(void);
static esp_err_t switch_to_mode(wifi_manager_mode_t new_mode);

esp_err_t wifi_manager_init(void)
{
    if (wifi_manager_initialized)
    {
        ESP_LOGW(TAG, "WiFi manager already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing WiFi manager with smart boot logic");

    // Initialize NVS if not already done
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP adapter
    ESP_ERROR_CHECK(esp_netif_init());

    // Create event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create event group
    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL)
    {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    // Create network interfaces
    wifi_netif_sta = esp_netif_create_default_wifi_sta();
    wifi_netif_ap = esp_netif_create_default_wifi_ap();

    // Initialize WiFi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Load stored credentials
    load_credentials_from_nvs();

    // Create connection timeout timer
    esp_timer_create_args_t timer_args = {
        .callback = connection_timeout_callback,
        .arg = NULL,
        .name = "wifi_timeout"
    };
    
    esp_err_t timer_ret = esp_timer_create(&timer_args, &connection_timeout_timer);
    if (timer_ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to create connection timeout timer: %s", esp_err_to_name(timer_ret));
    }

    wifi_manager_initialized = true;
    ESP_LOGI(TAG, "WiFi manager initialized successfully");
    ESP_LOGI(TAG, "Stored credentials: %s", has_stored_credentials ? "YES" : "NO");

    return ESP_OK;
}

esp_err_t wifi_manager_start(void)
{
    if (!wifi_manager_initialized)
    {
        ESP_LOGE(TAG, "WiFi manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Starting WiFi manager with smart boot logic");

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Smart boot logic: try STA mode first if we have credentials
    if (has_stored_credentials)
    {
        ESP_LOGI(TAG, "Attempting to connect to stored network: %s", stored_credentials.ssid);
        return switch_to_mode(WIFI_MODE_STA_CONNECTING);
    }
    else
    {
        ESP_LOGI(TAG, "No stored credentials, starting in AP mode");
        return switch_to_mode(WIFI_MODE_AP_ACTIVE);
    }
}

esp_err_t wifi_manager_stop(void)
{
    if (!wifi_manager_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Stopping WiFi manager");

    // Stop and delete timeout timer
    if (connection_timeout_timer) {
        esp_timer_stop(connection_timeout_timer);
        esp_timer_delete(connection_timeout_timer);
        connection_timeout_timer = NULL;
    }

    // Stop web server first
    web_server_stop();

    // Stop WiFi
    esp_wifi_stop();

    current_mode = WIFI_MODE_DISCONNECTED;
    retry_count = 0;

    ESP_LOGI(TAG, "WiFi manager stopped");
    return ESP_OK;
}

esp_err_t wifi_manager_get_status(wifi_status_t *status)
{
    if (status == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    status->mode = current_mode;
    status->retry_count = retry_count;
    status->has_credentials = has_stored_credentials;
    status->rssi = 0; // TODO: Get actual RSSI

    // Copy connected SSID
    if (current_mode == WIFI_MODE_STA_CONNECTED)
    {
        strncpy(status->connected_ssid, stored_credentials.ssid, sizeof(status->connected_ssid) - 1);
        status->connected_ssid[sizeof(status->connected_ssid) - 1] = '\0';
    }
    else
    {
        status->connected_ssid[0] = '\0';
    }

    return ESP_OK;
}

esp_err_t wifi_manager_set_credentials(const wifi_credentials_t *credentials)
{
    if (credentials == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Setting new WiFi credentials for SSID: %s", credentials->ssid);

    // Save to NVS first
    esp_err_t ret = save_credentials_to_nvs(credentials);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save credentials to NVS");
        return ret;
    }

    // Update local copy
    stored_credentials = *credentials;
    has_stored_credentials = true;
    retry_count = 0;

    // Attempt to connect
    return switch_to_mode(WIFI_MODE_STA_CONNECTING);
}

esp_err_t wifi_manager_clear_credentials(void)
{
    ESP_LOGI(TAG, "Clearing stored WiFi credentials");

    // Clear from NVS
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK)
    {
        nvs_erase_key(nvs_handle, NVS_SSID_KEY);
        nvs_erase_key(nvs_handle, NVS_PASSWORD_KEY);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }

    // Clear local copy
    memset(&stored_credentials, 0, sizeof(stored_credentials));
    has_stored_credentials = false;
    retry_count = 0;

    // Switch to AP mode
    return switch_to_mode(WIFI_MODE_AP_ACTIVE);
}

esp_err_t wifi_manager_get_ip_address(char *ip_str, size_t max_len)
{
    if (ip_str == NULL || max_len < 16)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = (current_mode == WIFI_MODE_STA_CONNECTED) ? wifi_netif_sta : wifi_netif_ap;

    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
    {
        snprintf(ip_str, max_len, IPSTR, IP2STR(&ip_info.ip));
        return ESP_OK;
    }

    return ESP_ERR_INVALID_STATE;
}

esp_err_t wifi_manager_switch_to_ap(void)
{
    ESP_LOGI(TAG, "Manual switch to AP mode");
    return switch_to_mode(WIFI_MODE_AP_ACTIVE);
}

// Private functions

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WiFi STA started, connecting...");
            
            // Start connection timeout timer
            if (connection_timeout_timer) {
                esp_timer_start_once(connection_timeout_timer, DEFAULT_WIFI_STA_TIMEOUT_MS * 1000); // Convert to microseconds
            }
            
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "WiFi STA connected to AP, waiting for IP...");
            
            // Stop timeout timer since we're connected at WiFi layer
            if (connection_timeout_timer) {
                esp_timer_stop(connection_timeout_timer);
            }
            
            // Start a longer timeout for IP acquisition (30 seconds)
            if (connection_timeout_timer) {
                esp_timer_start_once(connection_timeout_timer, 30000 * 1000); // 30 seconds for IP
            }
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
        {
            wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
            ESP_LOGW(TAG, "WiFi STA disconnected (reason: %d)", event->reason);

            if (current_mode == WIFI_MODE_STA_CONNECTING || current_mode == WIFI_MODE_STA_CONNECTED)
            {
                if (retry_count < DEFAULT_WIFI_STA_MAX_RETRY)
                {
                    retry_count++;
                    ESP_LOGI(TAG, "Retry connecting to WiFi (%d/%d)", retry_count, DEFAULT_WIFI_STA_MAX_RETRY);
                    esp_wifi_connect();
                }
                else
                {
                    ESP_LOGW(TAG, "Failed to connect after %d retries, switching to AP mode", DEFAULT_WIFI_STA_MAX_RETRY);
                    // Switch back to AP mode
                    switch_to_mode(WIFI_MODE_AP_ACTIVE);
                    xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                }
            }
            break;
        }

        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "WiFi AP started: %s", DEFAULT_WIFI_AP_SSID);
            current_mode = WIFI_MODE_AP_ACTIVE;

            // Start web server for captive portal
            web_server_config_t web_config = WEB_SERVER_DEFAULT_CONFIG();
            if (web_server_init(&web_config) == ESP_OK)
            {
                web_server_start();
                ESP_LOGI(TAG, "Captive portal started at 192.168.4.1");
            }
            break;

        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "WiFi AP stopped");
            web_server_stop();
            break;

        case WIFI_EVENT_AP_STACONNECTED:
        {
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(TAG, "Client connected to AP, MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                     event->mac[0], event->mac[1], event->mac[2],
                     event->mac[3], event->mac[4], event->mac[5]);
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
            ESP_LOGI(TAG, "Client disconnected from AP, MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                     event->mac[0], event->mac[1], event->mac[2],
                     event->mac[3], event->mac[4], event->mac[5]);
            break;
        }
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "WiFi connected! IP: " IPSTR, IP2STR(&event->ip_info.ip));

            // Stop any running timeout timer
            if (connection_timeout_timer) {
                esp_timer_stop(connection_timeout_timer);
            }

            current_mode = WIFI_MODE_STA_CONNECTED;
            retry_count = 0;
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);

            // Start web server on STA network
            web_server_config_t web_config = WEB_SERVER_DEFAULT_CONFIG();
            if (web_server_init(&web_config) == ESP_OK)
            {
                web_server_start();
            }
        }
    }
}

static void connection_timeout_callback(void* arg)
{
    ESP_LOGW(TAG, "Connection timeout after %d ms, switching to AP mode", DEFAULT_WIFI_STA_TIMEOUT_MS);
    
    // Check current WiFi state
    wifi_ap_record_t ap_info;
    esp_err_t wifi_status = esp_wifi_sta_get_ap_info(&ap_info);
    
    if (wifi_status == ESP_OK) {
        ESP_LOGW(TAG, "WiFi layer is connected to %s (RSSI: %d), but no IP received", 
                 ap_info.ssid, ap_info.rssi);
        ESP_LOGW(TAG, "This might be a DHCP issue on the network");
    } else {
        ESP_LOGW(TAG, "WiFi layer also disconnected");
    }
    
    // Cancel the timer
    if (connection_timeout_timer) {
        esp_timer_stop(connection_timeout_timer);
    }
    
    // Force switch to AP mode
    switch_to_mode(WIFI_MODE_AP_ACTIVE);
}

static esp_err_t load_credentials_from_nvs(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGD(TAG, "No stored WiFi credentials found");
        return ret;
    }

    size_t ssid_len = sizeof(stored_credentials.ssid);
    size_t password_len = sizeof(stored_credentials.password);

    ret = nvs_get_str(nvs_handle, NVS_SSID_KEY, stored_credentials.ssid, &ssid_len);
    if (ret == ESP_OK)
    {
        ret = nvs_get_str(nvs_handle, NVS_PASSWORD_KEY, stored_credentials.password, &password_len);
        if (ret == ESP_OK)
        {
            has_stored_credentials = true;
            ESP_LOGI(TAG, "Loaded stored credentials for SSID: %s", stored_credentials.ssid);
        }
    }

    nvs_close(nvs_handle);
    return ret;
}

esp_err_t wifi_manager_monitor(void)
{
    static int64_t last_log_time = 0;
    const int64_t LOG_INTERVAL_US = 30 * 1000 * 1000; // 30 seconds in microseconds
    
    int64_t current_time = esp_timer_get_time();
    
    // Periodic WiFi status logging (every 30 seconds)
    if (current_time - last_log_time >= LOG_INTERVAL_US)
    {
        wifi_status_t wifi_status;
        if (wifi_manager_get_status(&wifi_status) == ESP_OK)
        {
            const char *mode_str = "";
            switch (wifi_status.mode)
            {
            case WIFI_MODE_DISCONNECTED:
                mode_str = "Disconnected";
                break;
            case WIFI_MODE_STA_CONNECTING:
                mode_str = "Connecting";
                break;
            case WIFI_MODE_STA_CONNECTED:
                mode_str = "Connected (STA)";
                break;
            case WIFI_MODE_AP_ACTIVE:
                mode_str = "Access Point";
                break;
            case WIFI_MODE_SWITCHING:
                mode_str = "Switching";
                break;
            default:
                mode_str = "Unknown";
                break;
            }

            char ip_str[16] = "N/A";
            wifi_manager_get_ip_address(ip_str, sizeof(ip_str));

            ESP_LOGI(TAG, "WiFi Status: %s | IP: %s | SSID: %s",
                     mode_str, ip_str,
                     wifi_status.connected_ssid[0] ? wifi_status.connected_ssid : "N/A");
        }
        last_log_time = current_time;
    }
    
    return ESP_OK;
}

static esp_err_t save_credentials_to_nvs(const wifi_credentials_t *credentials)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = nvs_set_str(nvs_handle, NVS_SSID_KEY, credentials->ssid);
    if (ret == ESP_OK)
    {
        ret = nvs_set_str(nvs_handle, NVS_PASSWORD_KEY, credentials->password);
    }

    if (ret == ESP_OK)
    {
        ret = nvs_commit(nvs_handle);
    }

    nvs_close(nvs_handle);
    return ret;
}

static esp_err_t start_sta_mode(void)
{
    ESP_LOGI(TAG, "Starting STA mode");

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, stored_credentials.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, stored_credentials.password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    // Debug: Log credentials being used
    ESP_LOGI(TAG, "STA connecting to SSID: '%s', password: '%s'", wifi_config.sta.ssid, wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    current_mode = WIFI_MODE_STA_CONNECTING;
    retry_count = 0;

    return ESP_OK;
}

static esp_err_t start_ap_mode(void)
{
    ESP_LOGI(TAG, "Starting AP mode: %s", DEFAULT_WIFI_AP_SSID);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = DEFAULT_WIFI_AP_SSID,
            .ssid_len = strlen(DEFAULT_WIFI_AP_SSID),
            .channel = DEFAULT_WIFI_AP_CHANNEL,
            .password = DEFAULT_WIFI_AP_PASSWORD,
            .max_connection = DEFAULT_WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_OPEN},
    };

    // Debug: Log the actual SSID being set
    ESP_LOGI(TAG, "AP SSID length: %d, content: '%s'", wifi_config.ap.ssid_len, wifi_config.ap.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    return ESP_OK;
}

static esp_err_t switch_to_mode(wifi_manager_mode_t new_mode)
{
    if (new_mode == current_mode)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Switching WiFi mode: %d -> %d", current_mode, new_mode);
    current_mode = WIFI_MODE_SWITCHING;

    // Stop web server during transition
    web_server_stop();

    esp_err_t ret = ESP_OK;

    switch (new_mode)
    {
    case WIFI_MODE_STA_CONNECTING:
        if (has_stored_credentials)
        {
            ret = start_sta_mode();
        }
        else
        {
            ESP_LOGW(TAG, "No credentials for STA mode, switching to AP");
            ret = start_ap_mode();
        }
        break;

    case WIFI_MODE_AP_ACTIVE:
        ret = start_ap_mode();
        break;

    default:
        ESP_LOGE(TAG, "Invalid target mode: %d", new_mode);
        ret = ESP_ERR_INVALID_ARG;
        break;
    }

    return ret;
}