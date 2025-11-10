/**
 * @file display_logic.c
 * @brief LED Distance Visualization business logic.
 *
 * Design is documented in `docs/design/display-design.md` and linked to requirements
 * and design IDs (e.g. REQ_DISPLAY_*, DSN_DISPLAY_*). Keep this header short; full design
 * rationale lives in the design document for traceability.
 * 
 * CONFIGURATION:
 * Uses new JSON-based config_manager API to read distance parameters:
 * - dist_min_mm: Minimum distance threshold (millimeters)
 * - dist_max_mm: Maximum distance threshold (millimeters)
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "led_controller.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

/**
 * @brief Update LED display based on distance measurement
 *
 * @param measurement Distance measurement from sensor
 */
static void update_led_display(const distance_measurement_t *measurement)
{
    // Get configuration from JSON config manager - REQ_DISPLAY_2
    int32_t dist_min_mm = 100;   // Default minimum distance (100mm = 10cm)
    int32_t dist_max_mm = 500;   // Default maximum distance (500mm = 50cm)
    
    // Read from config manager with fallback to defaults
    if (config_get_int32("dist_min_mm", &dist_min_mm) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read dist_min_mm, using default 100");
        dist_min_mm = 100;
    }
    if (config_get_int32("dist_max_mm", &dist_max_mm) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read dist_max_mm, using default 500");
        dist_max_mm = 500;
    }
    
    // Clear all LEDs first
    led_clear_all();

    switch (measurement->status)
    {
    case DISTANCE_SENSOR_OK:
    {
        // Normal measurement - calculate LED position from distance
        if (measurement->distance_mm >= dist_min_mm && measurement->distance_mm <= dist_max_mm)
        {
            // Normal range: Calculate LED position directly with integer arithmetic
            uint32_t range_mm = dist_max_mm - dist_min_mm;
            uint32_t offset_mm = measurement->distance_mm - dist_min_mm;
            
            uint16_t led_count = led_get_count();
            // Use integer math with multiplication before division for precision
            uint32_t led_index = (offset_mm * (led_count - 1)) / range_mm;
            
            // Ensure within bounds
            if (led_index >= led_count) led_index = led_count - 1;
            
            // Normal range: Green color for distance visualization - REQ_DISPLAY_3
            led_color_t color = LED_COLOR_GREEN;
            led_set_pixel((uint16_t)led_index, color);
            ESP_LOGD(TAG, "Distance %d mm → LED %" PRIu32, measurement->distance_mm, led_index);
        }
        else if (measurement->distance_mm < dist_min_mm)
        {
            // Too close: Red on first LED - REQ_DISPLAY_4
            led_set_pixel(0, LED_COLOR_RED);
            ESP_LOGD(TAG, "Distance %d mm too close (min=%" PRIi32 ") → LED 0 red", 
                     measurement->distance_mm, dist_min_mm);
        }
        else
        {
            // Too far: Red on last LED - REQ_DISPLAY_5
            uint16_t led_count = led_get_count();
            led_set_pixel(led_count - 1, LED_COLOR_RED);
            ESP_LOGD(TAG, "Distance %d mm too far (max=%" PRIi32 ") → LED %d red", 
                     measurement->distance_mm, dist_max_mm, led_count - 1);
        }
        break;

    }

    case DISTANCE_SENSOR_TIMEOUT:
        // Sensor timeout: All LEDs off (already cleared)
        ESP_LOGD(TAG, "Sensor timeout → All LEDs off");
        break;

    case DISTANCE_SENSOR_OUT_OF_RANGE:
        // Out of sensor range: Red on last LED
        {
            uint16_t led_count = led_get_count();
            led_set_pixel(led_count - 1, LED_COLOR_RED);
            ESP_LOGD(TAG, "Sensor out of range → LED %d red", led_count - 1);
        }
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

    // Get current configuration from JSON config manager to log the range
    int32_t dist_min_mm = 100;
    int32_t dist_max_mm = 500;
    config_get_int32("dist_min_mm", &dist_min_mm);
    config_get_int32("dist_max_mm", &dist_max_mm);
    
    ESP_LOGI(TAG, "Distance range: %" PRIi32 "-%" PRIi32 "mm → LEDs 0-%d, blocking until new measurements",
             dist_min_mm, dist_max_mm, led_get_count() - 1);

    distance_measurement_t measurement;

    while (1)
    {
        // This will now BLOCK until new measurement arrives
        if (distance_sensor_get_latest(&measurement) == ESP_OK)
        {
            update_led_display(&measurement);

            ESP_LOGD(TAG, "Processed distance: %d mm, status: %d",
                     measurement.distance_mm, measurement.status);
        }
        // No delay needed - function blocks until next measurement
    }
}

esp_err_t display_logic_start(void)
{
    if (display_task_handle != NULL)
    {
        ESP_LOGW(TAG, "Display logic task already running");
        return ESP_ERR_INVALID_STATE;
    }

    // Configuration is stored in JSON config_manager - no pre-validation needed
    // Configuration parameters are read dynamically during display update

    // Check if LED controller is initialized
    if (!led_is_initialized())
    {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Check if distance sensor is running
    if (!distance_sensor_is_running())
    {
        ESP_LOGW(TAG, "Distance sensor not running. Display may not update.");
    }

    // Get LED count for logging
    uint16_t led_count = led_get_count();
    int32_t dist_min_mm = 100;
    int32_t dist_max_mm = 500;
    config_get_int32("dist_min_mm", &dist_min_mm);
    config_get_int32("dist_max_mm", &dist_max_mm);

    ESP_LOGI(TAG, "Display logic initialized successfully");
    ESP_LOGI(TAG, "Config: %" PRIi32 "-%" PRIi32 "mm → LEDs 0-%d",
             dist_min_mm, dist_max_mm, led_count - 1);

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

// Legacy functions removed for architectural simplification:
// - display_logic_get_config(): Configuration access now via config_manager API (REQ-CFG-2)
// - display_logic_stop(): Restart-based architecture pattern
// - display_logic_is_running(): Simplified lifecycle management