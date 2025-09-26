/**
 * @file wifi_manager_sim.c
 * @brief Simulator stub for wifi_manager - provides same API without using WiFi/NVS
 *
 * This implementation is selected during emulator builds and avoids calling
 * hardware/network drivers. It emulates basic behavior for the rest of the
 * application (init/start/stop/get_status/set_credentials/etc.) without
 * side effects.
 */

#include "wifi_manager.h"
#include "esp_err.h"
#include <string.h>
#include "esp_timer.h"

static wifi_manager_mode_t sim_mode = WIFI_MODE_DISCONNECTED;
static wifi_credentials_t sim_credentials = {0};
static char sim_ip[16] = "192.168.4.2"; // default AP-like address

esp_err_t wifi_manager_init(void)
{
    sim_mode = WIFI_MODE_DISCONNECTED;
    return ESP_OK;
}

esp_err_t wifi_manager_start(void)
{
    // In emulator we just enter AP mode so web server can operate
    sim_mode = WIFI_MODE_AP_ACTIVE;
    return ESP_OK;
}

esp_err_t wifi_manager_stop(void)
{
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
