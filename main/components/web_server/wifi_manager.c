/**
 * @file wifi_manager_new.c
 * @brief Ultra-Simplified WiFi Manager - Pure Restart-Based Approach
 * 
 * DESIGN PHILOSOPHY:
 * - No dynamic switching, no complex state management
 * - Pure restart-based approach with single NVS boot mode flag
 * - Only 2 code paths: STA boot or AP boot
 * - Web server always starts in clean context
 * - Bulletproof recovery, can't get stuck anywhere
 * 
 * BOOT LOGIC PSEUDO-CODE:
 * 
 * boot_mode = nvs_get("boot_mode"); // "STA" or "AP", default "STA"
 * 
 * if (boot_mode == "STA" || boot_mode_missing) {
 *     // STA MODE BOOT
 *     load_credentials_from_nvs();
 *     start_sta_mode();
 *     start_10s_timeout(); // → set_boot_mode("AP") + restart
 *     // Success → start_web_server() + stay_forever
 * }
 * else if (boot_mode == "AP") {
 *     // AP MODE BOOT  
 *     set_boot_mode("STA");        // FIRST THING - prepare escape route
 *     start_ap_mode();
 *     start_web_server();          // Always works - clean AP context
 *     start_10min_timeout();       // → restart (into STA mode)
 *     // User credentials → restart (into STA mode)
 * }
 * 
 * // User credential changes (ANY mode)
 * wifi_set_credentials() {
 *     save_credentials_to_nvs();
 *     esp_restart();               // Don't touch boot_mode flag!
 * }
 * 
 * FLOW:
 * Boot → Check boot_mode flag
 * ├─ "STA" → Try STA → Success (web server) OR Fail (10s → AP restart)
 * └─ "AP"  → Set "STA" flag → AP mode + web server → Timeout/User (restart to STA)
 * 
 * STATE MANAGEMENT:
 * - One NVS flag: boot_mode ("STA" or "AP")
 * - STA failure: Sets flag to "AP"
 * - AP startup: Sets flag to "STA" (immediate escape route)
 * - User action: Just restart (let boot logic handle it)
 */

#include "wifi_manager.h"
#include "web_server.h"
#include "config_manager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "string.h"

static const char *TAG = "wifi_manager";

// NVS storage keys
#define NVS_NAMESPACE "wifi_config"
#define NVS_SSID_KEY "ssid"
#define NVS_PASSWORD_KEY "password"
#define NVS_BOOT_MODE_KEY "boot_mode"

// Boot mode values
#define BOOT_MODE_STA "STA"
#define BOOT_MODE_AP "AP"

// Timing constants
#define STA_TIMEOUT_MS (10 * 1000)      // 10 seconds STA timeout
#define AP_TIMEOUT_MS (10 * 60 * 1000)  // 10 minutes AP timeout
#define RESTART_DELAY_MS (3 * 1000)     // 3 seconds before restart

// Default AP configuration
#define DEFAULT_WIFI_AP_SSID "ESP32-Setup"
#define DEFAULT_WIFI_AP_PASSWORD "12345678"
#define DEFAULT_WIFI_AP_CHANNEL 1
#define DEFAULT_WIFI_AP_MAX_CONN 4

// Global state (minimal)
static bool wifi_initialized = false;
static wifi_manager_mode_t current_mode = WIFI_MODE_DISCONNECTED;
static wifi_credentials_t stored_credentials = {0};
static esp_timer_handle_t timeout_timer = NULL;
static esp_timer_handle_t restart_timer = NULL;
static esp_netif_t *netif_sta = NULL;
static esp_netif_t *netif_ap = NULL;
static EventGroupHandle_t wifi_event_group = NULL;

// Event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

// Function declarations
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void timeout_callback(void* arg);
static void restart_callback(void* arg);
static esp_err_t load_credentials_from_nvs(void);
static esp_err_t save_boot_mode(const char* mode);
static esp_err_t get_boot_mode(char* mode, size_t max_len);
static esp_err_t start_sta_boot(void);
static esp_err_t start_ap_boot(void);

//=============================================================================
// PUBLIC API FUNCTIONS
//=============================================================================

esp_err_t wifi_manager_init(void)
{
    if (wifi_initialized) {
        ESP_LOGW(TAG, "WiFi manager already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing ultra-simplified WiFi manager");

    // Initialize NVS if needed
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize network interfaces
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    netif_sta = esp_netif_create_default_wifi_sta();
    netif_ap = esp_netif_create_default_wifi_ap();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Create event group
    wifi_event_group = xEventGroupCreate();
    if (!wifi_event_group) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Create timers
    esp_timer_create_args_t timeout_args = {
        .callback = timeout_callback,
        .arg = NULL,
        .name = "wifi_timeout"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timeout_args, &timeout_timer));

    esp_timer_create_args_t restart_args = {
        .callback = restart_callback,
        .arg = NULL,
        .name = "wifi_restart"
    };
    ESP_ERROR_CHECK(esp_timer_create(&restart_args, &restart_timer));

    wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi manager initialized successfully");
    return ESP_OK;
}

esp_err_t wifi_manager_start(void)
{
    if (!wifi_initialized) {
        ESP_LOGE(TAG, "WiFi manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Starting ultra-simplified WiFi manager");

    // Load stored credentials
    load_credentials_from_nvs();

    // Check boot mode flag
    char boot_mode[8] = {0};
    esp_err_t ret = get_boot_mode(boot_mode, sizeof(boot_mode));
    
    if (ret != ESP_OK || strcmp(boot_mode, BOOT_MODE_AP) != 0) {
        // Default to STA mode (first boot or explicit STA mode)
        ESP_LOGI(TAG, "Booting in STA mode (default or explicit)");
        return start_sta_boot();
    } else {
        // Boot in AP mode
        ESP_LOGI(TAG, "Booting in AP mode");
        return start_ap_boot();
    }
}

esp_err_t wifi_manager_stop(void)
{
    if (!wifi_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Stopping WiFi manager");
    
    // Stop timers
    if (timeout_timer) {
        esp_timer_stop(timeout_timer);
        esp_timer_delete(timeout_timer);
        timeout_timer = NULL;
    }
    if (restart_timer) {
        esp_timer_stop(restart_timer);
        esp_timer_delete(restart_timer);
        restart_timer = NULL;
    }

    // Stop web server and WiFi
    web_server_stop();
    esp_wifi_stop();
    
    current_mode = WIFI_MODE_DISCONNECTED;
    ESP_LOGI(TAG, "WiFi manager stopped");
    return ESP_OK;
}

esp_err_t wifi_manager_get_status(wifi_status_t *status)
{
    if (!status) {
        return ESP_ERR_INVALID_ARG;
    }

    status->mode = current_mode;
    status->retry_count = 0; // Not used in restart-based approach
    status->has_credentials = (stored_credentials.ssid[0] != '\0');
    status->rssi = 0; // TODO: Get actual RSSI

    if (current_mode == WIFI_MODE_STA_CONNECTED) {
        strncpy(status->connected_ssid, stored_credentials.ssid, sizeof(status->connected_ssid) - 1);
        status->connected_ssid[sizeof(status->connected_ssid) - 1] = '\0';
    } else {
        status->connected_ssid[0] = '\0';
    }

    return ESP_OK;
}

esp_err_t wifi_manager_set_credentials(const wifi_credentials_t *credentials)
{
    if (!credentials) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Setting new WiFi credentials for SSID: %s", credentials->ssid);

    // Save credentials using config_manager (uses no_commit + commit pattern)
    esp_err_t ret = config_set_string_no_commit("wifi_ssid", credentials->ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set wifi_ssid: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = config_set_string_no_commit("wifi_pass", credentials->password);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set wifi_pass: %s", esp_err_to_name(ret));
        return ret;
    }

    // Commit all changes at once
    ret = config_commit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit WiFi credentials: %s", esp_err_to_name(ret));
        return ret;
    }

    // Update local copy
    stored_credentials = *credentials;

    ESP_LOGI(TAG, "Credentials saved, restarting in %d seconds...", RESTART_DELAY_MS / 1000);
    
    // Restart to apply new credentials (boot mode flag determines STA/AP)
    esp_timer_start_once(restart_timer, RESTART_DELAY_MS * 1000);
    
    return ESP_OK;
}

esp_err_t wifi_manager_clear_credentials(void)
{
    ESP_LOGI(TAG, "Clearing stored WiFi credentials");

    // Clear from NVS
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK) {
        nvs_erase_key(nvs_handle, NVS_SSID_KEY);
        nvs_erase_key(nvs_handle, NVS_PASSWORD_KEY);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }

    // Clear local copy
    memset(&stored_credentials, 0, sizeof(stored_credentials));

    ESP_LOGI(TAG, "Credentials cleared, restarting in %d seconds...", RESTART_DELAY_MS / 1000);
    
    // Restart (will try STA with empty credentials → fail → AP mode)
    esp_timer_start_once(restart_timer, RESTART_DELAY_MS * 1000);
    
    return ESP_OK;
}

// Legacy function - simplified to just restart
esp_err_t wifi_manager_switch_to_ap(void)
{
    ESP_LOGI(TAG, "Manual switch to AP mode - setting boot mode and restarting...");
    
    save_boot_mode(BOOT_MODE_AP);
    esp_timer_start_once(restart_timer, RESTART_DELAY_MS * 1000);
    
    return ESP_OK;
}

esp_err_t wifi_manager_get_ip_address(char *ip_str, size_t max_len)
{
    if (!ip_str || max_len < 16) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = (current_mode == WIFI_MODE_STA_CONNECTED) ? netif_sta : netif_ap;

    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        snprintf(ip_str, max_len, IPSTR, IP2STR(&ip_info.ip));
        return ESP_OK;
    }

    return ESP_ERR_INVALID_STATE;
}

//=============================================================================
// PRIVATE HELPER FUNCTIONS
//=============================================================================

static esp_err_t load_credentials_from_nvs(void)
{
    // Load credentials from config_manager
    esp_err_t ret = config_get_string("wifi_ssid", stored_credentials.ssid, sizeof(stored_credentials.ssid));
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "No stored WiFi SSID found");
        return ret;
    }

    ret = config_get_string("wifi_pass", stored_credentials.password, sizeof(stored_credentials.password));
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "No stored WiFi password found");
        // Clear SSID if password is missing
        stored_credentials.ssid[0] = '\0';
        return ret;
    }

    ESP_LOGI(TAG, "Loaded stored credentials for SSID: %s", stored_credentials.ssid);
    return ESP_OK;
}

static esp_err_t save_boot_mode(const char* mode)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS for boot mode");
        return ret;
    }

    ret = nvs_set_str(nvs_handle, NVS_BOOT_MODE_KEY, mode);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_handle);
    }
    
    nvs_close(nvs_handle);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Boot mode set to: %s", mode);
    }
    
    return ret;
}

static esp_err_t get_boot_mode(char* mode, size_t max_len)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "No boot mode found in NVS, defaulting to STA");
        return ret;
    }

    size_t required_size = max_len;
    ret = nvs_get_str(nvs_handle, NVS_BOOT_MODE_KEY, mode, &required_size);
    nvs_close(nvs_handle);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Boot mode from NVS: %s", mode);
    }
    
    return ret;
}

static esp_err_t start_sta_boot(void)
{
    ESP_LOGI(TAG, "=== STA BOOT MODE ===");
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Configure STA mode with stored credentials (even if empty)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, stored_credentials.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, stored_credentials.password, sizeof(wifi_config.sta.password) - 1);
    
    // Add configuration for better WPA2 handshake compatibility
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    
    ESP_LOGI(TAG, "Attempting STA connection to: '%s'", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    current_mode = WIFI_MODE_STA_CONNECTING;
    
    // Start timeout timer (10 seconds)
    esp_timer_start_once(timeout_timer, STA_TIMEOUT_MS * 1000);
    
    return ESP_OK;
}

static esp_err_t start_ap_boot(void)
{
    ESP_LOGI(TAG, "=== AP BOOT MODE ===");
    
    // FIRST THING: Set boot mode to STA (prepare escape route)
    save_boot_mode(BOOT_MODE_STA);
    ESP_LOGI(TAG, "Boot mode set to STA for next restart");
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Configure AP mode
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    
    // Load AP configuration from config_manager
    char ap_ssid[33] = {0};
    char ap_pass[64] = {0};
    
    esp_err_t ret = config_get_string("ap_ssid", ap_ssid, sizeof(ap_ssid));
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to load ap_ssid, using default: %s", DEFAULT_WIFI_AP_SSID);
        strncpy(ap_ssid, DEFAULT_WIFI_AP_SSID, sizeof(ap_ssid) - 1);
    }
    
    ret = config_get_string("ap_pass", ap_pass, sizeof(ap_pass));
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to load ap_pass, using default");
        strncpy(ap_pass, DEFAULT_WIFI_AP_PASSWORD, sizeof(ap_pass) - 1);
    }
    
    // Configure AP with loaded settings
    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(ap_ssid),
            .channel = DEFAULT_WIFI_AP_CHANNEL,
            .max_connection = DEFAULT_WIFI_AP_MAX_CONN,
            .authmode = (strlen(ap_pass) >= 8) ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN
        }
    };
    
    // Copy SSID and password to config structure
    strncpy((char *)wifi_config.ap.ssid, ap_ssid, sizeof(wifi_config.ap.ssid));
    strncpy((char *)wifi_config.ap.password, ap_pass, sizeof(wifi_config.ap.password));
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_LOGI(TAG, "AP mode configured: %s (auth: %s)", ap_ssid, 
             wifi_config.ap.authmode == WIFI_AUTH_OPEN ? "Open" : "WPA2");
    
    current_mode = WIFI_MODE_AP_ACTIVE;
    
    // Start web server for AP mode
    web_server_config_t web_config = WEB_SERVER_DEFAULT_CONFIG();
    if (web_server_init(&web_config) == ESP_OK) {
        web_server_start();
        ESP_LOGI(TAG, "Web server started on 192.168.4.1");
    } else {
        ESP_LOGE(TAG, "Failed to start web server in AP mode");
    }
    
    // Start timeout timer (10 minutes)
    esp_timer_start_once(timeout_timer, AP_TIMEOUT_MS * 1000);
    
    return ESP_OK;
}

//=============================================================================
// EVENT HANDLERS AND CALLBACKS
//=============================================================================

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started, connecting...");
                esp_wifi_connect();
                break;
                
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi STA connected, waiting for IP...");
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGW(TAG, "WiFi STA disconnected (reason: %d)", event->reason);
                
                if (current_mode == WIFI_MODE_STA_CONNECTING || current_mode == WIFI_MODE_STA_CONNECTED) {
                    if (event->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT || 
                        event->reason == WIFI_REASON_HANDSHAKE_TIMEOUT) {
                        ESP_LOGW(TAG, "Handshake timeout - will retry connection");
                        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait 1 second
                        esp_wifi_connect(); // Immediate retry
                    } else {
                        ESP_LOGW(TAG, "STA connection failed, will timeout and restart to AP mode");
                        // Let timeout handle the restart for other reasons
                    }
                }
                break;
            }
            
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "WiFi AP started successfully");
                // Web server is started in start_ap_boot() function, not here
                break;
                
            default:
                break;
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "WiFi connected! IP: " IPSTR, IP2STR(&event->ip_info.ip));
        
        // Stop timeout timer - we're fully connected
        esp_timer_stop(timeout_timer);
        
        current_mode = WIFI_MODE_STA_CONNECTED;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        
        // Start web server for STA mode - we have an IP now!
        web_server_config_t web_config = WEB_SERVER_DEFAULT_CONFIG();
        if (web_server_init(&web_config) == ESP_OK) {
            web_server_start();
            ESP_LOGI(TAG, "Web server started on network IP");
        } else {
            ESP_LOGE(TAG, "Failed to start web server in STA mode");
        }
    }
}

static void timeout_callback(void* arg)
{
    if (current_mode == WIFI_MODE_STA_CONNECTING) {
        ESP_LOGW(TAG, "STA connection timeout (%d seconds) - switching to AP mode", STA_TIMEOUT_MS / 1000);
        
        // Set boot mode to AP and restart
        save_boot_mode(BOOT_MODE_AP);
        esp_timer_start_once(restart_timer, RESTART_DELAY_MS * 1000);
        
    } else if (current_mode == WIFI_MODE_AP_ACTIVE) {
        ESP_LOGI(TAG, "AP timeout (%d minutes) - restarting to try STA mode", AP_TIMEOUT_MS / (60 * 1000));
        
        // Boot mode already set to STA in start_ap_boot(), just restart
        esp_timer_start_once(restart_timer, RESTART_DELAY_MS * 1000);
    }
}

static void restart_callback(void* arg)
{
    ESP_LOGI(TAG, "Restarting device for WiFi mode change...");
    esp_restart();
}

// Minimal monitoring function (legacy compatibility)
esp_err_t wifi_manager_monitor(void)
{
    // In restart-based approach, monitoring is minimal
    // Real monitoring happens through event handlers
    return ESP_OK;
}
