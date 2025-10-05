/**
 * @file wifi_manager_sim.c
 * @brief Simulator stub for wifi_manager - uses UART IP tunnel instead of WiFi
 *
 * This implementation is selected during emulator builds and uses a UART-based
 * IP tunnel to provide network connectivity in QEMU without WiFi emulation.
 */

#include "wifi_manager.h"
#include "web_server.h"
#include "netif_uart_tunnel_sim.h"
#include "esp_err.h"
#include "esp_log.h"
#include <string.h>
#include "esp_timer.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"

static const char *TAG = "wifi_manager_sim";

static wifi_manager_mode_t sim_mode = WIFI_MODE_DISCONNECTED;
static wifi_credentials_t sim_credentials = {0};
static char sim_ip[16] = "192.168.100.2"; // IP address for UART tunnel
static bool netif_initialized = false;

esp_err_t wifi_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing WiFi manager simulator");
    
    // Initialize NVS (may already be done, but safe to call multiple times)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    // Initialize network interface layer (required for HTTP server)
    if (!netif_initialized) {
        ESP_LOGI(TAG, "Initializing network interface layer");
        ret = esp_netif_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize netif: %s", esp_err_to_name(ret));
            return ret;
        }
        
        // Initialize event loop (required for HTTP server)
        ret = esp_event_loop_create_default();
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "Failed to create event loop: %s", esp_err_to_name(ret));
            return ret;
        }
        
        netif_initialized = true;
        ESP_LOGI(TAG, "Network interface layer initialized");
    }
    
    sim_mode = WIFI_MODE_DISCONNECTED;
    ESP_LOGI(TAG, "WiFi manager simulator initialized successfully");
    return ESP_OK;
}

esp_err_t wifi_manager_start(void)
{
    ESP_LOGI(TAG, "Starting WiFi manager simulator with UART IP tunnel");
    
    // Initialize UART-based IP tunnel
    netif_uart_tunnel_config_t tunnel_config = {
        .hostname = "esp32-distance",
        .ip_addr = {192, 168, 100, 2},
        .netmask = {255, 255, 255, 0},
        .gateway = {192, 168, 100, 1}
    };
    
    esp_err_t ret = netif_uart_tunnel_init(&tunnel_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UART tunnel: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Update simulated mode
    sim_mode = WIFI_MODE_STA_CONNECTED;  // Pretend we're connected
    
    // Start the webserver
    ESP_LOGI(TAG, "Starting web server on UART tunnel interface");
    web_server_config_t web_config = WEB_SERVER_DEFAULT_CONFIG();
    ret = web_server_init(&web_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize web server: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = web_server_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start web server: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "UART tunnel active: IP %s, access via host TUN device", sim_ip);
    return ESP_OK;
}

esp_err_t wifi_manager_stop(void)
{
    ESP_LOGI(TAG, "Stopping WiFi manager simulator");
    web_server_stop();
    netif_uart_tunnel_deinit();
    sim_mode = WIFI_MODE_DISCONNECTED;
    return ESP_OK;
}

esp_err_t wifi_manager_get_status(wifi_status_t *status)
{
    if (!status) return ESP_ERR_INVALID_ARG;
    status->mode = sim_mode;
    status->rssi = 0;
    status->retry_count = 0;
    status->has_credentials = (sim_credentials.ssid[0] != '\0');
    if (sim_mode == WIFI_MODE_STA_CONNECTED) {
        strncpy(status->connected_ssid, sim_credentials.ssid, sizeof(status->connected_ssid)-1);
        status->connected_ssid[sizeof(status->connected_ssid)-1] = '\0';
    } else {
        status->connected_ssid[0] = '\0';
    }
    return ESP_OK;
}

esp_err_t wifi_manager_set_credentials(const wifi_credentials_t *credentials)
{
    if (!credentials) return ESP_ERR_INVALID_ARG;
    // Copy but do not attempt real connection in emulator
    strncpy(sim_credentials.ssid, credentials->ssid, sizeof(sim_credentials.ssid)-1);
    strncpy(sim_credentials.password, credentials->password, sizeof(sim_credentials.password)-1);
    sim_credentials.ssid[sizeof(sim_credentials.ssid)-1] = '\0';
    sim_credentials.password[sizeof(sim_credentials.password)-1] = '\0';
    // emulate a restart-to-apply by switching mode to AP (no real restart)
    sim_mode = WIFI_MODE_AP_ACTIVE;
    return ESP_OK;
}

esp_err_t wifi_manager_clear_credentials(void)
{
    memset(&sim_credentials, 0, sizeof(sim_credentials));
    sim_mode = WIFI_MODE_AP_ACTIVE;
    return ESP_OK;
}

esp_err_t wifi_manager_get_ip_address(char *ip_str, size_t max_len)
{
    if (!ip_str || max_len < sizeof(sim_ip)) return ESP_ERR_INVALID_ARG;
    strncpy(ip_str, sim_ip, max_len-1);
    ip_str[max_len-1] = '\0';
    return ESP_OK;
}

esp_err_t wifi_manager_switch_to_ap(void)
{
    sim_mode = WIFI_MODE_AP_ACTIVE;
    return ESP_OK;
}

esp_err_t wifi_manager_monitor(void)
{
    // No-op for emulator
    return ESP_OK;
}
