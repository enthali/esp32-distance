#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "led_controller.h"
#include "test/test_task.h"
#include "test/led_running_test.h"
#include "distance_sensor.h"
#include "wifi_manager.h"
#include "display_logic.h"
#include "config.h"
#include "config_manager.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Distance Measurement with LED Strip Display");

    // Initialize configuration management first
    esp_err_t ret = config_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize configuration manager: %s", esp_err_to_name(ret));
        esp_restart();
    }

    // Load current runtime configuration
    system_config_t runtime_config;
    ret = config_get_current(&runtime_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get current configuration: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ESP_LOGI(TAG, "Configuration loaded: LED count=%d, brightness=%d, dist_range=%.1f-%.1fcm", 
             runtime_config.led_count, runtime_config.led_brightness,
             runtime_config.distance_min_cm, runtime_config.distance_max_cm);

    // Perform system health check (REQ-CFG-11)
    size_t nvs_free, nvs_total;
    esp_err_t health_ret = config_nvs_health_check(&nvs_free, &nvs_total);
    if (health_ret == ESP_OK) {
        ESP_LOGI(TAG, "System health check passed - NVS: %zu/%zu entries", nvs_total - nvs_free, nvs_total);
    } else {
        ESP_LOGW(TAG, "System health check issues detected: %s", esp_err_to_name(health_ret));
    }

    // Configure LED strip using runtime configuration
    led_config_t led_config = {
        .gpio_pin = LED_DATA_PIN,
        .led_count = runtime_config.led_count,
        .rmt_channel = LED_RMT_CHANNEL};

    // Initialize LED controller
    ret = led_controller_init(&led_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize LED controller: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ESP_LOGI(TAG, "LED controller initialized successfully");
    ESP_LOGI(TAG, "LED count: %d", led_get_count());

    // Clear all LEDs first
    led_clear_all();
    led_show();

    // Run hardware test once at startup (not continuous background task)
    ESP_LOGI(TAG, "Running one-time LED hardware test...");
    led_running_test_single_cycle(LED_COLOR_GREEN, 50);
    ESP_LOGI(TAG, "Hardware test completed");

    // Clear LEDs after test
    led_clear_all();
    led_show();

    // Configure and initialize distance sensor using runtime configuration
    distance_sensor_config_t distance_config = {
        .trigger_pin = DISTANCE_TRIGGER_PIN,
        .echo_pin = DISTANCE_ECHO_PIN,
        .measurement_interval_ms = runtime_config.measurement_interval_ms,
        .timeout_ms = runtime_config.sensor_timeout_ms,
        .temperature_celsius = runtime_config.temperature_c,
        .smoothing_alpha = runtime_config.smoothing_alpha};

    ret = distance_sensor_init(&distance_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize distance sensor: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ret = distance_sensor_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start distance sensor: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ESP_LOGI(TAG, "Distance sensor initialized and started");
    ESP_LOGI(TAG, "Hardware: LED=GPIO%d, Trigger=GPIO%d, Echo=GPIO%d", LED_DATA_PIN, DISTANCE_TRIGGER_PIN, DISTANCE_ECHO_PIN);

    // Initialize and start WiFi manager with smart boot logic
    ret = wifi_manager_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize WiFi manager: %s", esp_err_to_name(ret));
        return;
    }

    ret = wifi_manager_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start WiFi manager: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "WiFi manager initialized and started");
    ESP_LOGI(TAG, "Ready for distance measurement, LED display, and web interface...");

    // Configure and initialize display logic using runtime configuration
    display_config_t display_config = {
        .min_distance_cm = runtime_config.distance_min_cm,
        .max_distance_cm = runtime_config.distance_max_cm};

    ret = display_logic_init(&display_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize display logic: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ret = display_logic_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start display logic: %s", esp_err_to_name(ret));
        esp_restart();
    }

    ESP_LOGI(TAG, "Display logic initialized and started");
    ESP_LOGI(TAG, "Ready for distance measurement and LED display...");

    // Main application loop - Coordination and lightweight monitoring
    while (1)
    {
        // Lightweight sensor health monitoring 
        distance_sensor_monitor();
        
        // Periodic WiFi status logging 
        wifi_manager_monitor();

        vTaskDelay(pdMS_TO_TICKS(5000)); // Monitor loop every 5 seconds
    }
}
