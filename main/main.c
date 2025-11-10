/**
 * @file main.c
 * @brief ESP32 Distance Sensor - Application Entry Point
 * 
 * This application provides:
 * - Distance measurement with HC-SR04 ultrasonic sensor
 * - Real-time LED strip visualization (WS2812)
 * - Web interface with captive portal for configuration
 * - WiFi connectivity (STA mode with AP fallback)
 * - JSON-based configuration management
 * 
 * SYSTEM ARCHITECTURE:
 * - Distance Sensor Task (priority 5): Continuous HC-SR04 measurements
 * - Display Logic Task (priority 3): LED visualization from measurements  
 * - Web Server Task: HTTP interface for config and monitoring
 * - WiFi Manager Task: Network connectivity management
 * 
 * REQUIREMENTS TRACEABILITY:
 * - REQ_DISPLAY_1: WS2812 LED strip support
 * - REQ_DISTANCE_SENSOR_1: HC-SR04 measurements
 * - REQ_CONFIG_JSON_1: JSON schema-based configuration
 * - REQ_STARTUP_2: Visual boot sequence
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

// Application components - Distance Sensor System
#include "led_controller.h"
#include "led_running_test.h"
#include "distance_sensor.h"
#include "display_logic.h"
#include "config_manager.h"

// Web interface components
#include "wifi_manager.h"

// Hardware pin definitions (project-specific configuration)
#define LED_DATA_PIN        GPIO_NUM_19    // WS2812 data line
#define DISTANCE_TRIGGER_PIN GPIO_NUM_14   // HC-SR04 trigger
#define DISTANCE_ECHO_PIN    GPIO_NUM_13   // HC-SR04 echo

static const char *TAG = "main";

/**
 * @brief Main application entry point
 * 
 * Initializes the ESP32 distance sensor system with web configuration interface.
 * System starts with LED boot sequence, then initializes distance measurement,
 * LED visualization, and web configuration interface.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║    ESP32 Distance Sensor - Starting...         ║");
    ESP_LOGI(TAG, "║    WiFi + Web Config + LED Visualization       ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    // Step 1: Initialize configuration manager
    // Internally initializes NVS and loads configuration from storage
    // Uses JSON schema from config_schema.json for all parameters
    ESP_LOGI(TAG, "Initializing configuration manager...");
    ESP_ERROR_CHECK(config_init());
    ESP_LOGI(TAG, "✓ Configuration manager initialized (NVS ready)");
    
    // Step 2: Initialize LED controller
    // REQ_DISPLAY_1: WS2812 LED strip support
    // Configuration (led_count, brightness) loaded from config_manager
    ESP_LOGI(TAG, "Initializing LED controller...");
    ESP_ERROR_CHECK(led_controller_init(LED_DATA_PIN));
    ESP_LOGI(TAG, "✓ LED controller initialized (%d LEDs on GPIO%d)", 
             led_get_count(), LED_DATA_PIN);
    
    // Step 3: Run startup test sequence
    // REQ_STARTUP_2: Visual boot sequence - demonstrates all LEDs working
    ESP_LOGI(TAG, "Running LED startup test...");
    led_clear_all();
    led_show();
    ESP_ERROR_CHECK(led_running_test_single_cycle(LED_COLOR_GREEN, 50));
    led_clear_all();
    led_show();
    ESP_LOGI(TAG, "✓ LED startup test completed");
    
    // Step 4: Initialize distance sensor
    // REQ_DISTANCE_SENSOR_1: HC-SR04 ultrasonic measurements
    // Configuration (interval, timeout, etc.) loaded from config_manager
    ESP_LOGI(TAG, "Initializing distance sensor...");
    ESP_ERROR_CHECK(distance_sensor_init(DISTANCE_TRIGGER_PIN, DISTANCE_ECHO_PIN));
    ESP_ERROR_CHECK(distance_sensor_start());
    ESP_LOGI(TAG, "✓ Distance sensor initialized and started");
    ESP_LOGI(TAG, "  Hardware: Trigger=GPIO%d, Echo=GPIO%d", 
             DISTANCE_TRIGGER_PIN, DISTANCE_ECHO_PIN);
    
    // Step 5: Initialize WiFi manager and web server
    // Handles both STA mode (connect to WiFi) and AP mode (captive portal)
    // WiFi manager automatically starts web server in both modes:
    //   - AP mode: Web server on 192.168.4.1 (captive portal)
    //   - STA mode: Web server on network IP (after connection)
    ESP_LOGI(TAG, "Initializing WiFi manager...");
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(wifi_manager_start());
    ESP_LOGI(TAG, "✓ WiFi manager initialized (web server lifecycle managed automatically)");
    
    // Step 6: Start display logic
    // REQ_DISPLAY_1 + REQ_DISPLAY_3: LED visualization of distance measurements
    ESP_LOGI(TAG, "Starting display logic...");
    ESP_ERROR_CHECK(display_logic_start());
    ESP_LOGI(TAG, "✓ Display logic started - monitoring distance measurements");
    
    // System initialized successfully
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║          System Ready!                     ║");
    ESP_LOGI(TAG, "║  Distance: Monitoring                      ║");
    ESP_LOGI(TAG, "║  LED Display: READY                        ║");
    ESP_LOGI(TAG, "║  Web Interface: http://192.168.4.1         ║");
    ESP_LOGI(TAG, "║  Captive Portal: Auto (AP mode)            ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "");
    
    // Main monitoring loop
    // Lightweight periodic health checks and logging
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10 second interval
        
        // Monitor system health
        distance_sensor_monitor();  // Check for queue overflows
        wifi_manager_monitor();      // Check WiFi connection status
        
        // Log system metrics
        uint32_t heap_free = esp_get_free_heap_size();
        uint32_t heap_min = esp_get_minimum_free_heap_size();
        uint32_t uptime_s = (uint32_t)(esp_timer_get_time() / 1000000);
        
        ESP_LOGD(TAG, "Uptime: %lu s | Heap: %lu/%lu bytes | Overflows: %lu",
                 uptime_s, heap_free, heap_min,
                 distance_sensor_get_queue_overflows());
    }
}
