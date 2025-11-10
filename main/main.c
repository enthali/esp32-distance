/**
 * @file main.c
 * @brief ESP32 Template - Application Entry Point with Web Configuration
 * 
 * This template provides:
 * - WiFi connectivity (STA mode with AP fallback)
 * - Web-based configuration interface (captive portal)
 * - Configuration management (NVS storage)
 * - QEMU network support (UART tunnel for emulation)
 * 
 * Customize the web interface and add your application logic.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"

// Template components
#include "config_manager.h"
#include "web_server.h"
#include "wifi_manager.h"

#ifdef CONFIG_IDF_TARGET_ESP32
    // QEMU network support (only for emulator)
    // Note: wifi_manager_sim.c handles netif_uart_tunnel initialization
#endif

static const char *TAG = "main";

/**
 * @brief Main application entry point
 * 
 * Initializes the ESP32 template with web-based configuration.
 * The device will:
 * 1. Try to connect to saved WiFi (STA mode)
 * 2. Fall back to AP mode with captive portal if connection fails
 * 3. Provide web interface for configuration and monitoring
 */
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Template Starting...");
    ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
    
    // Initialize NVS (Non-Volatile Storage)
    // Required for WiFi credentials and configuration storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated and needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS Flash initialized successfully");
    
    // Initialize configuration manager
    ESP_LOGI(TAG, "Initializing configuration manager...");
    ESP_ERROR_CHECK(config_init());
    
#ifdef CONFIG_IDF_TARGET_ESP32
    // QEMU Build: Use simulator WiFi manager with UART tunnel
    // The WiFi manager simulator will initialize the web server internally
    ESP_LOGI(TAG, "Initializing WiFi manager (QEMU/simulator mode)...");
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(wifi_manager_start());
#else
    // Real Hardware: Initialize web server with WiFi manager
    ESP_LOGI(TAG, "Initializing web server...");
    web_server_config_t web_config = WEB_SERVER_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(web_server_init(&web_config));
    ESP_ERROR_CHECK(web_server_start());
#endif
    
    ESP_LOGI(TAG, "Template initialized successfully");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "  ESP32 Project Template - Web Configured");
    ESP_LOGI(TAG, "  ");
    ESP_LOGI(TAG, "  Access web interface:");
    ESP_LOGI(TAG, "  - STA mode: http://<device-ip>");
    ESP_LOGI(TAG, "  - AP mode:  http://192.168.4.1");
    ESP_LOGI(TAG, "  - QEMU:     http://localhost:8080");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "");
    
    // Main application loop
    // Add your custom application logic here
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        
        // Example: Monitor system health
        ESP_LOGI(TAG, "System uptime: %lu seconds, Free heap: %lu bytes",
                 (unsigned long)(esp_timer_get_time() / 1000000),
                 (unsigned long)esp_get_free_heap_size());
    }
}
