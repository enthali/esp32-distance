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
 * - temp_cold_c10: Cold temperature threshold in 0.1°C (default 50 = 5.0°C)
 * - temp_warm_c10: Warm temperature threshold in 0.1°C (default 200 = 20.0°C)
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "temp_sensor.h"
#include "led_controller.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

// Configuration parameters (read at initialization per REQ_DSP_2)
static int32_t dist_min_mm    = 100;  // Default 100mm = 10cm
static int32_t dist_max_mm    = 500;  // Default 500mm = 50cm
static int32_t temp_cold_c10  = 50;   // Default  5.0°C (blue)
static int32_t temp_warm_c10  = 200;  // Default 20.0°C (orange)

/**
 * @brief Compute position LED colour from ambient temperature (REQ_DSP_6, SPEC_DSP_ALGO_1).
 *
 * Gradient: blue (0,0,255) → green (0,255,0) → orange (255,165,0)
 * Two-segment linear interpolation between temp_cold_c10 and temp_warm_c10.
 * If no valid temperature reading is available, returns green (original behaviour).
 *
 * @param color  Output colour.
 */
static void get_temperature_colour(led_color_t *color)
{
    temp_measurement_t meas;
    if (temp_sensor_get_latest(&meas) != ESP_OK || !meas.valid) {
        /* No valid reading — fall back to green (REQ_DSP_6 AC-5) */
        *color = LED_COLOR_GREEN;
        return;
    }

    int32_t temp = (int32_t)meas.temperature_c_x10;
    int32_t cold = temp_cold_c10;
    int32_t warm = temp_warm_c10;

    if (temp <= cold) {
        /* At or below cold threshold → blue */
        color->red = 0; color->green = 0; color->blue = 255;
        return;
    }
    if (temp >= warm) {
        /* At or above warm threshold → orange */
        color->red = 255; color->green = 165; color->blue = 0;
        return;
    }

    /* Between thresholds: two-segment blend (t ∈ [0, 1000])
     * 0–500: blue  → green
     * 500–1000: green → orange                    */
    int32_t range = warm - cold;  /* > 0 guaranteed by config validation */
    int32_t t = (temp - cold) * 1000 / range;
    /* Clamp for safety */
    if (t < 0)    t = 0;
    if (t > 1000) t = 1000;

    if (t <= 500) {
        color->red = 0;
        color->green = (uint8_t)(t * 255 / 500);
        color->blue = (uint8_t)((500 - t) * 255 / 500);
    } else {
        int32_t t2 = t - 500;
        color->red = (uint8_t)(t2 * 255 / 500);
        color->green = (uint8_t)(255 - t2 * (255 - 165) / 500);
        color->blue = 0;
    }
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
            
            // Temperature-based colour for position indicator (REQ_DSP_3 AC-6, REQ_DSP_6)
            led_color_t color;
            get_temperature_colour(&color);
            led_set_pixel((uint16_t)led_index, color);
            ESP_LOGD(TAG, "Distance %d mm → LED %" PRIu32 " (r=%d g=%d b=%d)",
                     measurement->distance_mm, led_index, color.red, color.green, color.blue);
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

    // Configuration already loaded at initialization (REQ_DSP_2 AC-2)
    ESP_LOGI(TAG, "Distance range: %" PRIi32 "-%" PRIi32 "mm → LEDs 0-%d",
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

    // Read and cache configuration at initialization (REQ_DSP_2 AC-2)
    if (config_get_int32("dist_min_mm", &dist_min_mm) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read dist_min_mm from config, using default 100mm");
        dist_min_mm = 100;
    }
    if (config_get_int32("dist_max_mm", &dist_max_mm) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read dist_max_mm from config, using default 500mm");
        dist_max_mm = 500;
    }
    // Temperature colour thresholds (REQ_DSP_6 AC-4, REQ_CFG_JSON_16)
    if (config_get_int32("temp_cold_c10", &temp_cold_c10) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read temp_cold_c10 from config, using default 50 (5.0°C)");
        temp_cold_c10 = 50;
    }
    if (config_get_int32("temp_warm_c10", &temp_warm_c10) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read temp_warm_c10 from config, using default 200 (20.0°C)");
        temp_warm_c10 = 200;
    }
    /* Validate thresholds: cold must be < warm (REQ_CFG_JSON_16 AC-6) */
    if (temp_cold_c10 >= temp_warm_c10) {
        ESP_LOGW(TAG, "temp_cold_c10 (%ld) >= temp_warm_c10 (%ld), resetting to defaults",
                 (long)temp_cold_c10, (long)temp_warm_c10);
        temp_cold_c10 = 50;
        temp_warm_c10 = 200;
    }

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

    ESP_LOGI(TAG, "Display logic initialized successfully");
    ESP_LOGI(TAG, "Distance range: %" PRIi32 "-%" PRIi32 "mm → LEDs 0-%d",
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