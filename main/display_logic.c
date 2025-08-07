/**
 * @file display_logic.c
 * @brief LED Distance Visualization Business Logic Implementation
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "led_controller.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

// Configuration
static display_config_t display_config;
static bool is_initialized = false;

/**
 * @brief Map distance to LED index
 *
 * Maps distance in centimeters to LED index (0 to led_count-1).
 * Uses linear mapping: configured distance range mapped to all available LEDs.
 *
 * @param distance_cm Distance in centimeters
 * @return LED index (0 to led_count-1), or -1 if out of normal range
 */
static int map_distance_to_led(float distance_cm)
{
    if (distance_cm < display_config.min_distance_cm || distance_cm > display_config.max_distance_cm)
    {
        return -1; // Out of range
    }

    // Linear mapping: Distance range → LEDs 0 to (led_count-1)
    float range_cm = display_config.max_distance_cm - display_config.min_distance_cm;
    float normalized = (distance_cm - display_config.min_distance_cm) / range_cm;     // 0.0-1.0
    
    // Get actual LED count for dynamic mapping
    uint16_t led_count = led_get_count();
    int led_index = (int)(normalized * (float)(led_count - 1));                      // 0 to (led_count-1)

    // Ensure within bounds
    if (led_index < 0)
        led_index = 0;
    if (led_index >= led_count)
        led_index = led_count - 1;

    return led_index;
}

/**
 * @brief Update LED display based on distance measurement
 *
 * @param measurement Distance measurement from sensor
 */
static void update_led_display(const distance_measurement_t *measurement)
{
    // Clear all LEDs first
    led_clear_all();

    switch (measurement->status)
    {
    case DISTANCE_SENSOR_OK:
    {
        int led_index = map_distance_to_led(measurement->distance_cm);

        if (led_index >= 0)
        {
            // Normal range: Green color for distance visualization
            led_color_t color = LED_COLOR_GREEN;
            led_set_pixel(led_index, color);
            ESP_LOGD(TAG, "Distance %.2f cm → LED %d", measurement->distance_cm, led_index);
        }
        else if (measurement->distance_cm < display_config.min_distance_cm)
        {
            // Too close: Red on first LED
            led_set_pixel(0, LED_COLOR_RED);
            ESP_LOGD(TAG, "Distance %.2f cm too close → LED 0 red", measurement->distance_cm);
        }
        else
        {
            // Too far: Red on last LED
            led_set_pixel(39, LED_COLOR_RED);
            ESP_LOGD(TAG, "Distance %.2f cm too far → LED 39 red", measurement->distance_cm);
        }
        break;
    }

    case DISTANCE_SENSOR_TIMEOUT:
        // Sensor timeout: All LEDs off (already cleared)
        ESP_LOGD(TAG, "Sensor timeout → All LEDs off");
        break;

    case DISTANCE_SENSOR_OUT_OF_RANGE:
        // Out of sensor range: Red on last LED
        led_set_pixel(39, LED_COLOR_RED);
        ESP_LOGD(TAG, "Sensor out of range → LED 39 red");
        break;

    case DISTANCE_SENSOR_NO_ECHO:
    case DISTANCE_SENSOR_INVALID_READING:
    default:
        // Other errors: Red on first LED
        led_set_pixel(0, LED_COLOR_RED);
        ESP_LOGD(TAG, "Sensor error → LED 0 red");
        break;
    }

    // Update physical LEDs
    led_show();
}

/**
 * @brief Main display logic task function
 *
 * Continuously waits for distance measurements and updates LED display.
 * Runs at priority 3 (between distance sensor priority 6 and test priority 2).
 * Blocks on distance_sensor_get_latest() until new measurements arrive.
 */
static void display_logic_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Display logic task started (Priority: %d, Core: %d)",
             uxTaskPriorityGet(NULL), xPortGetCoreID());

    ESP_LOGI(TAG, "Distance range: %.1f-%.1fcm → LEDs 0-39, blocking until new measurements",
             display_config.min_distance_cm, display_config.max_distance_cm);

    distance_measurement_t measurement;

    while (1)
    {
        // This will now BLOCK until new measurement arrives
        if (distance_sensor_get_latest(&measurement) == ESP_OK)
        {
            update_led_display(&measurement);

            ESP_LOGD(TAG, "Processed distance: %.2f cm, status: %d",
                     measurement.distance_cm, measurement.status);
        }
        // No delay needed - function blocks until next measurement
    }
}

esp_err_t display_logic_init(const display_config_t *config)
{
    if (is_initialized)
    {
        ESP_LOGW(TAG, "Display logic already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Config parameter is required
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Configuration parameter is required");
        return ESP_ERR_INVALID_ARG;
    }

    // Copy provided configuration
    display_config = *config;

    // Validate configuration
    if (display_config.min_distance_cm >= display_config.max_distance_cm)
    {
        ESP_LOGE(TAG, "Invalid distance range: min=%.1f, max=%.1f",
                 display_config.min_distance_cm, display_config.max_distance_cm);
        return ESP_ERR_INVALID_ARG;
    }

    // Check if LED controller is initialized
    if (!led_is_initialized())
    {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Verify LED count matches configuration expectation
    uint16_t led_count = led_get_count();
    if (led_count != DEFAULT_LED_COUNT)
    {
        ESP_LOGW(TAG, "Expected %d LEDs, found %d. Mapping will be adjusted dynamically.", DEFAULT_LED_COUNT, led_count);
    }

    is_initialized = true;

    ESP_LOGI(TAG, "Display logic initialized successfully");
    ESP_LOGI(TAG, "Config: %.1f-%.1fcm → LEDs 0-%d",
             display_config.min_distance_cm, display_config.max_distance_cm, led_count - 1);

    return ESP_OK;
}

esp_err_t display_logic_start(void)
{
    if (!is_initialized)
    {
        ESP_LOGE(TAG, "Display logic not initialized. Call display_logic_init() first.");
        return ESP_ERR_INVALID_STATE;
    }

    if (display_task_handle != NULL)
    {
        ESP_LOGW(TAG, "Display logic task already running");
        return ESP_ERR_INVALID_STATE;
    }

    // Check if distance sensor is running
    if (!distance_sensor_is_running())
    {
        ESP_LOGW(TAG, "Distance sensor not running. Display may not update.");
    }

    // Create display logic task
    BaseType_t result = xTaskCreatePinnedToCore(
        display_logic_task,     // Task function
        "display_logic",        // Task name
        4096,                   // Stack size (4KB)
        NULL,                   // Parameters
        3,                      // Priority 3 (between distance sensor 6 and test 2)
        &display_task_handle,   // Task handle
        1                       // Core ID (run on core 1)
    );

    if (result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create display logic task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Display logic task started successfully");
    return ESP_OK;
}

esp_err_t display_logic_stop(void)
{
    if (display_task_handle == NULL)
    {
        ESP_LOGW(TAG, "Display logic task not running");
        return ESP_ERR_INVALID_STATE;
    }

    // Delete task
    vTaskDelete(display_task_handle);
    display_task_handle = NULL;

    // Clear all LEDs
    led_clear_all();
    led_show();

    ESP_LOGI(TAG, "Display logic task stopped");
    return ESP_OK;
}

bool display_logic_is_running(void)
{
    return (display_task_handle != NULL);
}

display_config_t display_logic_get_config(void)
{
    return display_config;
}