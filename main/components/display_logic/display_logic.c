/**
 * @file display_logic.c
 * @brief LED Distance Visualization business logic.
 *
 * Design is documented in `docs/design/display-design.md` and linked to requirements
 * and design IDs (e.g. REQ-DSP-*, DSN-DSP-*). Keep this header short; full design
 * rationale lives in the design document for traceability.
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "led_controller.h"
#include "config.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

// Flash timer for below minimum display (REQ-DSP-VISUAL-03, DSN-DSP-TIMING-01)
static TimerHandle_t flash_timer_handle = NULL;
static bool flash_state = false; // true = LEDs on, false = LEDs off
static bool below_minimum_active = false; // tracks if we're in below minimum state

/**
 * @brief Timer callback for flashing below minimum LEDs
 * 
 * Implements REQ-DSP-VISUAL-03: Flash every 10th LED with 1-second interval
 * Design documented in DSN-DSP-TIMING-01
 *
 * @param xTimer Timer handle (unused)
 */
static void flash_timer_callback(TimerHandle_t xTimer)
{
    if (!below_minimum_active) {
        return; // Safety check - should not happen if timer lifecycle is correct
    }

    // Toggle flash state
    flash_state = !flash_state;
    
    // Clear all LEDs first
    led_clear_all();
    
    if (flash_state) {
        // Flash ON: Illuminate every 10th LED in red
        uint16_t led_count = led_get_count();
        for (uint16_t i = 0; i < led_count; i += 10) {
            led_set_pixel(i, LED_COLOR_RED);
        }
        ESP_LOGD(TAG, "Flash ON: Every 10th LED red (positions 0,10,20...)");
    } else {
        ESP_LOGD(TAG, "Flash OFF: All LEDs off");
    }
    
    // Update physical LEDs
    led_show();
}

/**
 * @brief Stop the flashing timer if active
 * 
 * Helper function to ensure timer is properly stopped when exiting below minimum state
 */
static void stop_flash_timer_if_active(void)
{
    if (below_minimum_active) {
        below_minimum_active = false;
        if (flash_timer_handle != NULL) {
            xTimerStop(flash_timer_handle, 0);
        }
        ESP_LOGD(TAG, "Stopped flashing timer");
    }
}

/**
 * @brief Update LED display based on distance measurement
 *
 * @param measurement Distance measurement from sensor
 */
static void update_led_display(const distance_measurement_t *measurement)
{
    // Get configuration once for entire function (optimization: avoid multiple config_get_current calls)
    system_config_t config;
    bool config_valid = (config_get_current(&config) == ESP_OK);
    
    // Clear all LEDs first
    led_clear_all();

    switch (measurement->status)
    {
    case DISTANCE_SENSOR_OK:
    {
        if (config_valid)
        {
            // Use config directly instead of calling map_distance_to_led (avoids duplicate config_get_current)
            if (measurement->distance_mm >= config.distance_min_mm && measurement->distance_mm <= config.distance_max_mm)
            {
                // Stop flashing timer if we were in below minimum state
                stop_flash_timer_if_active();
                
                // Normal range: Calculate LED position directly with integer arithmetic
                uint16_t range_mm = config.distance_max_mm - config.distance_min_mm;
                uint16_t offset_mm = measurement->distance_mm - config.distance_min_mm;
                
                uint16_t led_count = led_get_count();
                // Use integer math with multiplication before division for precision
                int led_index = (int)((offset_mm * (led_count - 1)) / range_mm);
                
                // Ensure within bounds
                if (led_index < 0) led_index = 0;
                if (led_index >= led_count) led_index = led_count - 1;
                
                // Normal range: Green color for distance visualization
                led_color_t color = LED_COLOR_GREEN;
                led_set_pixel(led_index, color);
                ESP_LOGD(TAG, "Distance %d mm → LED %d", measurement->distance_mm, led_index);
            }
            else if (measurement->distance_mm < config.distance_min_mm)
            {
                // Below minimum: Start flashing every 10th LED (REQ-DSP-VISUAL-03)
                if (!below_minimum_active) {
                    // Transition to below minimum state
                    below_minimum_active = true;
                    flash_state = true; // Start with LEDs on
                    
                    // Start the flash timer (500ms period for 1-second cycle)
                    if (flash_timer_handle != NULL) {
                        xTimerStart(flash_timer_handle, 0);
                    }
                    
                    // Immediately show the first flash state
                    uint16_t led_count = led_get_count();
                    for (uint16_t i = 0; i < led_count; i += 10) {
                        led_set_pixel(i, LED_COLOR_RED);
                    }
                    ESP_LOGD(TAG, "Distance %d mm below minimum → Started flashing every 10th LED", measurement->distance_mm);
                }
                // If already in below minimum state, timer handles the flashing
            }
            else
            {
                // Stop flashing timer if we were in below minimum state
                stop_flash_timer_if_active();
                
                // Too far: Red on last LED
                uint16_t led_count = led_get_count();
                led_set_pixel(led_count - 1, LED_COLOR_RED);
                ESP_LOGD(TAG, "Distance %d mm too far → LED %d red", measurement->distance_mm, led_count - 1);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Failed to get configuration for display update");
        }
        break;
    }

    case DISTANCE_SENSOR_TIMEOUT:
        // Sensor timeout: All LEDs off (already cleared)
        stop_flash_timer_if_active();
        ESP_LOGD(TAG, "Sensor timeout → All LEDs off");
        break;

    case DISTANCE_SENSOR_OUT_OF_RANGE:
        // Out of sensor range: Red on last LED
        stop_flash_timer_if_active();
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
        stop_flash_timer_if_active();
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

    // Get current configuration to log the range
    system_config_t config;
    if (config_get_current(&config) == ESP_OK) {
        ESP_LOGI(TAG, "Distance range: %.1f-%.1fcm → LEDs 0-39, blocking until new measurements",
                 config.distance_min_mm / 10.0, config.distance_max_mm / 10.0);
    } else {
        ESP_LOGW(TAG, "Could not get configuration, using defaults");
    }

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

    // Get current configuration from config_manager (REQ-CFG-2) - consolidated from display_logic_init()
    system_config_t config;
    esp_err_t ret = config_get_current(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current configuration: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configuration validation is handled by config_manager - no redundant checks needed

    // Check if LED controller is initialized - consolidated from display_logic_init()
    if (!led_is_initialized())
    {
        ESP_LOGE(TAG, "LED controller not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Create flash timer for below minimum display (REQ-DSP-VISUAL-03, DSN-DSP-TIMING-01)
    if (flash_timer_handle == NULL) {
        flash_timer_handle = xTimerCreate(
            "flash_timer",          // Timer name
            pdMS_TO_TICKS(500),     // Period: 500ms for 1-second flash cycle
            pdTRUE,                 // Auto-reload: true
            NULL,                   // Timer ID (unused)
            flash_timer_callback    // Callback function
        );
        
        if (flash_timer_handle == NULL) {
            ESP_LOGE(TAG, "Failed to create flash timer");
            return ESP_ERR_NO_MEM;
        }
        ESP_LOGI(TAG, "Flash timer created successfully");
    }

    // Check if distance sensor is running
    if (!distance_sensor_is_running())
    {
        ESP_LOGW(TAG, "Distance sensor not running. Display may not update.");
    }

    // Get LED count for logging - consolidated from display_logic_init()
    uint16_t led_count = led_get_count();

    ESP_LOGI(TAG, "Display logic initialized successfully");
    ESP_LOGI(TAG, "Config: %.1f-%.1fcm → LEDs 0-%d",
             config.distance_min_mm / 10.0, config.distance_max_mm / 10.0, led_count - 1);

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
        
        // Cleanup timer on failure
        if (flash_timer_handle != NULL) {
            xTimerDelete(flash_timer_handle, 0);
            flash_timer_handle = NULL;
        }
        
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Display logic task started successfully");
    return ESP_OK;
}

// Legacy functions removed for architectural simplification:
// - display_logic_get_config(): Configuration access now via config_manager API (REQ-CFG-2)
// - display_logic_stop(): Restart-based architecture pattern
// - display_logic_is_running(): Simplified lifecycle management