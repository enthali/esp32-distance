// Auto-generated from config_schema.json - DO NOT EDIT MANUALLY
// Generator: tools/generate_config_factory.py

#include "config_manager.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

static const char *TAG = "config_factory";

/**
 * @brief Write factory default values to NVS
 * 
 * This function is auto-generated from config_schema.json.
 * It writes all default configuration values to NVS storage
 * using the bulk JSON API with structured format: [{key, type, value}, ...]
 */
void config_write_factory_defaults(void)
{
    ESP_LOGI(TAG, "Writing factory defaults to NVS...");

    // Structured JSON array with factory defaults
    const char *factory_json = "[{\"key\":\"wifi_ssid\",\"type\":\"string\",\"value\":\"\"},{\"key\":\"wifi_pass\",\"type\":\"password\",\"value\":\"\"},{\"key\":\"ap_ssid\",\"type\":\"string\",\"value\":\"ESP32-Setup\"},{\"key\":\"ap_pass\",\"type\":\"password\",\"value\":\"12345678\"},{\"key\":\"led_count\",\"type\":\"integer\",\"value\":40},{\"key\":\"led_bright\",\"type\":\"integer\",\"value\":128},{\"key\":\"dist_min_mm\",\"type\":\"integer\",\"value\":100},{\"key\":\"dist_max_mm\",\"type\":\"integer\",\"value\":500},{\"key\":\"meas_int_ms\",\"type\":\"integer\",\"value\":100},{\"key\":\"sens_timeout_ms\",\"type\":\"integer\",\"value\":30},{\"key\":\"temp_c_x10\",\"type\":\"integer\",\"value\":200},{\"key\":\"smooth_factor\",\"type\":\"integer\",\"value\":300},{\"key\":\"temp_gpio\",\"type\":\"integer\",\"value\":4},{\"key\":\"temp_intv_ms\",\"type\":\"integer\",\"value\":2000},{\"key\":\"temp_cold_c10\",\"type\":\"integer\",\"value\":50},{\"key\":\"temp_warm_c10\",\"type\":\"integer\",\"value\":200}]";

    esp_err_t ret = config_set_all_from_json(factory_json);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Factory defaults written successfully");
    } else {
        ESP_LOGE(TAG, "Failed to write factory defaults: %s", esp_err_to_name(ret));
    }
}

/**
 * @brief Check if all required configuration keys exist
 * 
 * This function is auto-generated from config_schema.json.
 * It verifies that all mandatory parameters are present in NVS.
 * If any are missing, it writes factory defaults and triggers a restart.
 * 
 * @return true if all keys exist, false if defaults were written and restart triggered
 */
bool config_validate_or_reset(void)
{
    ESP_LOGI(TAG, "Validating configuration completeness...");
    
    bool missing_keys = false;
    int32_t test_int;
    char test_str[64];
    
    if (config_get_string("wifi_ssid", test_str, sizeof(test_str)) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: wifi_ssid");
        missing_keys = true;
    }
    if (config_get_string("wifi_pass", test_str, sizeof(test_str)) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: wifi_pass");
        missing_keys = true;
    }
    if (config_get_string("ap_ssid", test_str, sizeof(test_str)) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: ap_ssid");
        missing_keys = true;
    }
    if (config_get_string("ap_pass", test_str, sizeof(test_str)) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: ap_pass");
        missing_keys = true;
    }
    if (config_get_int32("led_count", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: led_count");
        missing_keys = true;
    }
    if (config_get_int32("led_bright", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: led_bright");
        missing_keys = true;
    }
    if (config_get_int32("dist_min_mm", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: dist_min_mm");
        missing_keys = true;
    }
    if (config_get_int32("dist_max_mm", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: dist_max_mm");
        missing_keys = true;
    }
    if (config_get_int32("meas_int_ms", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: meas_int_ms");
        missing_keys = true;
    }
    if (config_get_int32("sens_timeout_ms", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: sens_timeout_ms");
        missing_keys = true;
    }
    if (config_get_int32("temp_c_x10", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: temp_c_x10");
        missing_keys = true;
    }
    if (config_get_int32("smooth_factor", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: smooth_factor");
        missing_keys = true;
    }
    if (config_get_int32("temp_gpio", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: temp_gpio");
        missing_keys = true;
    }
    if (config_get_int32("temp_intv_ms", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: temp_intv_ms");
        missing_keys = true;
    }
    if (config_get_int32("temp_cold_c10", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: temp_cold_c10");
        missing_keys = true;
    }
    if (config_get_int32("temp_warm_c10", &test_int) == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Missing key: temp_warm_c10");
        missing_keys = true;
    }
    
    if (missing_keys) {
        ESP_LOGW(TAG, "Configuration incomplete - writing factory defaults...");
        esp_err_t ret = config_factory_reset();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write factory defaults: %s", esp_err_to_name(ret));
            return false;
        }
        
        ESP_LOGI(TAG, "✓ Factory defaults written successfully");
        ESP_LOGI(TAG, "System will restart in 3 seconds to apply configuration...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_restart();
        return false;  // Never reached, but makes return path explicit
    }
    
    ESP_LOGI(TAG, "✓ Configuration validation passed");
    return true;
}
