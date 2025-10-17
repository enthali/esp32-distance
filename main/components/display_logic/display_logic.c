/**
 * @file display_logic.c
 * @brief LED Distance Visualization business logic with interactive animations.
 *
 * Design is documented in `docs/design/display-design.md` and linked to requirements
 * and design IDs (e.g. REQ-DSP-*, DSN-DSP-*). Keep this header short; full design
 * rationale lives in the design document for traceability.
 * 
 * Implements interactive LED animations for garage parking assistance:
 * - REQ-DSP-ANIM-01: Running light for "too far" zone
 * - REQ-DSP-ANIM-02: Running light for "too close" zone
 * - REQ-DSP-ANIM-03: Steady 4-LED display for ideal zone
 * - REQ-DSP-ANIM-04: Green LED for out-of-range (far)
 * - REQ-DSP-ANIM-05: Blinking pattern for emergency (too close)
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "led_controller.h"
#include "config.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

// Display state for animation management (REQ-DSP-IMPL-04)
static display_state_t display_state = {
    .mode = DISPLAY_MODE_STATIC,
    .current_position = 0,
    .target_position = 0,
    .measurement_position = 0,
    .last_update = 0,
    .blink_state = false,
    .step_delay_ms = ANIMATION_STEP_MIN_MS,
    .ideal_zone_start = 0,
    .ideal_zone_end = 0
};

/**
 * @brief Calculate dynamic ideal zone boundaries based on LED count
 * 
 * Implements REQ-DSP-ANIM-03 AC-5, AC-5a, AC-5b
 * 
 * @param led_count Total number of LEDs in strip
 * @param zone_start Output: start position of ideal zone
 * @param zone_end Output: end position of ideal zone
 */
static void calculate_ideal_zone(uint16_t led_count, uint8_t* zone_start, uint8_t* zone_end)
{
    // REQ-DSP-ANIM-03 AC-5a: Zone size = 10% of LEDs, minimum 4
    uint8_t zone_size = (led_count / 10 > 4) ? led_count / 10 : 4;
    
    // REQ-DSP-ANIM-03 AC-5b: Zone center at 25% position
    uint8_t zone_center = led_count / 4;
    
    *zone_start = zone_center - (zone_size / 2);
    *zone_end = *zone_start + zone_size - 1;
    
    ESP_LOGI(TAG, "Ideal zone for %d LEDs: %d-%d (%d LEDs at 25%% position)", 
             led_count, *zone_start, *zone_end, zone_size);
}

/**
 * @brief Determine display mode based on LED position with dynamic zones
 * 
 * Implements zone detection logic (DSN-DSP-ANIM-03)
 * 
 * @param led_index Calculated LED position from measurement
 * @return Display mode to use
 */
static display_mode_t determine_display_mode(int led_index)
{
    // Calculate ideal zone on first call or when LED count changes
    static uint16_t cached_led_count = 0;
    uint16_t current_led_count = led_get_count();
    
    if (cached_led_count != current_led_count) {
        calculate_ideal_zone(current_led_count, 
                            &display_state.ideal_zone_start,
                            &display_state.ideal_zone_end);
        cached_led_count = current_led_count;
    }
    
    // REQ-DSP-ANIM-03: Ideal zone (dynamic boundaries) → steady red LEDs
    if (led_index >= display_state.ideal_zone_start && 
        led_index <= display_state.ideal_zone_end) {
        return DISPLAY_MODE_IDEAL_STEADY;
    }
    // REQ-DSP-ANIM-02: Too close zone → running light toward ideal zone start
    else if (led_index < display_state.ideal_zone_start) {
        return DISPLAY_MODE_RUNNING_BACKWARD;
    }
    // REQ-DSP-ANIM-01: Too far zone → running light toward ideal zone end
    else {
        return DISPLAY_MODE_RUNNING_FORWARD;
    }
}

/**
 * @brief Render ideal zone display (steady LEDs)
 * 
 * Implements REQ-DSP-ANIM-03: Steady LED display for ideal zone with dynamic boundaries
 */
static void render_ideal_zone(void)
{
    led_clear_all();
    // Display steady RED LEDs in dynamically calculated ideal zone
    for (int i = display_state.ideal_zone_start; i <= display_state.ideal_zone_end; i++) {
        led_set_pixel(i, LED_COLOR_RED);
    }
    led_show();
}

/**
 * @brief Render running light animation
 * 
 * Implements DSN-DSP-ANIM-02: Running light animation algorithm
 * Used for both forward (too far) and backward (too close) animations.
 * 
 * @param color LED color for animation
 */
static void render_running_light(led_color_t color)
{
    led_clear_all();
    led_set_pixel(display_state.current_position, color);
    led_show();
}

/**
 * @brief Render blinking pattern for emergency mode
 * 
 * Implements DSN-DSP-ANIM-04: Blink pattern implementation (REQ-DSP-ANIM-05)
 */
static void render_blink_pattern(void)
{
    led_clear_all();
    if (display_state.blink_state) {
        // Blink LEDs at positions 0, 10, 20, 30 simultaneously
        uint16_t led_count = led_get_count();
        for (int i = 0; i < led_count; i += BLINK_LED_SPACING) {
            led_set_pixel(i, LED_COLOR_RED);
        }
    }
    led_show();
}

/**
 * @brief Calculate dynamic step delay for constant animation duration
 * 
 * Implements REQ-DSP-ANIM-01 AC-1a, AC-1b and REQ-DSP-ANIM-02 AC-1a, AC-1b
 * 
 * @param steps_to_target Number of LED steps to reach target
 * @return Step delay in milliseconds, clamped to reasonable range
 */
static uint32_t calculate_step_delay(uint32_t steps_to_target)
{
    if (steps_to_target == 0) {
        return ANIMATION_STEP_MIN_MS;
    }
    
    // REQ-DSP-ANIM-01 AC-1: Target duration ~1 second
    uint32_t step_delay_ms = ANIMATION_DURATION_MS / steps_to_target;
    
    // REQ-DSP-ANIM-01 AC-1b: Clamp to reasonable range
    if (step_delay_ms < ANIMATION_STEP_MIN_MS) {
        step_delay_ms = ANIMATION_STEP_MIN_MS;  // Max 50fps
    }
    if (step_delay_ms > ANIMATION_STEP_MAX_MS) {
        step_delay_ms = ANIMATION_STEP_MAX_MS;  // Min 5fps
    }
    
    return step_delay_ms;
}

/**
 * @brief Update animation state based on timing
 * 
 * Implements REQ-DSP-IMPL-04: Animation timing independent of measurement rate
 * Implements DSN-DSP-ANIM-02: Running light with dynamic timing
 */
static void update_animation_state(void)
{
    TickType_t current_time = xTaskGetTickCount();
    TickType_t elapsed = current_time - display_state.last_update;
    
    switch (display_state.mode) {
        case DISPLAY_MODE_RUNNING_FORWARD:
        case DISPLAY_MODE_RUNNING_BACKWARD:
            // REQ-DSP-ANIM-01, REQ-DSP-ANIM-02: Dynamic step delay for ~1 second animation
            if (elapsed >= pdMS_TO_TICKS(display_state.step_delay_ms)) {
                // Move animation position one step toward target
                if (display_state.current_position < display_state.target_position) {
                    display_state.current_position++;
                } else if (display_state.current_position > display_state.target_position) {
                    display_state.current_position--;
                } else {
                    // Reached target, loop back to measurement position
                    display_state.current_position = display_state.measurement_position;
                    
                    // Recalculate step delay for new loop (distance may have changed)
                    uint32_t steps = (display_state.current_position > display_state.target_position) ?
                                    display_state.current_position - display_state.target_position :
                                    display_state.target_position - display_state.current_position;
                    display_state.step_delay_ms = calculate_step_delay(steps);
                }
                display_state.last_update = current_time;
            }
            break;
            
        case DISPLAY_MODE_BLINK_PATTERN:
            // REQ-DSP-ANIM-05: 1 Hz (500ms per state)
            if (elapsed >= pdMS_TO_TICKS(BLINK_PERIOD_MS)) {
                display_state.blink_state = !display_state.blink_state;
                display_state.last_update = current_time;
            }
            break;
            
        default:
            // Static modes don't need animation updates
            break;
    }
}

/**
 * @brief Render current display state to LEDs
 * 
 * Implements DSN-DSP-ANIM-01: Animation state machine architecture
 */
static void render_display_state(void)
{
    switch (display_state.mode) {
        case DISPLAY_MODE_IDEAL_STEADY:
            render_ideal_zone();
            break;
            
        case DISPLAY_MODE_RUNNING_FORWARD:
            // REQ-DSP-ANIM-01: Green for "too far" zone
            render_running_light(LED_COLOR_GREEN);
            break;
            
        case DISPLAY_MODE_RUNNING_BACKWARD:
            // REQ-DSP-ANIM-02: Red for "too close" zone
            render_running_light(LED_COLOR_RED);
            break;
            
        case DISPLAY_MODE_BLINK_PATTERN:
            render_blink_pattern();
            break;
            
        case DISPLAY_MODE_STATIC:
        default:
            // Static display already rendered, just show
            led_show();
            break;
    }
}

/**
 * @brief Update LED display based on distance measurement with animations
 *
 * Implements REQ-DSP-ANIM-01 through REQ-DSP-ANIM-05
 * 
 * @param measurement Distance measurement from sensor
 */
static void update_led_display(const distance_measurement_t *measurement)
{
    // Get configuration once for entire function (optimization: avoid multiple config_get_current calls)
    system_config_t config;
    bool config_valid = (config_get_current(&config) == ESP_OK);
    
    switch (measurement->status)
    {
    case DISTANCE_SENSOR_OK:
    {
        if (config_valid)
        {
            // Use config directly instead of calling map_distance_to_led (avoids duplicate config_get_current)
            if (measurement->distance_mm >= config.distance_min_mm && measurement->distance_mm <= config.distance_max_mm)
            {
                // Normal range: Calculate LED position directly with integer arithmetic
                uint16_t range_mm = config.distance_max_mm - config.distance_min_mm;
                uint16_t offset_mm = measurement->distance_mm - config.distance_min_mm;
                
                uint16_t led_count = led_get_count();
                // Use integer math with multiplication before division for precision
                int led_index = (int)((offset_mm * (led_count - 1)) / range_mm);
                
                // Ensure within bounds
                if (led_index < 0) led_index = 0;
                if (led_index >= led_count) led_index = led_count - 1;
                
                // Determine display mode based on zone (DSN-DSP-ANIM-03)
                display_mode_t new_mode = determine_display_mode(led_index);
                
                // Update state if mode changed or first time
                if (display_state.mode != new_mode) {
                    display_state.mode = new_mode;
                    display_state.measurement_position = led_index;
                    display_state.current_position = led_index;
                    display_state.last_update = xTaskGetTickCount();
                    
                    // Set animation target based on mode using dynamic zone boundaries
                    if (new_mode == DISPLAY_MODE_RUNNING_FORWARD) {
                        // REQ-DSP-ANIM-01: Animate toward ideal zone end
                        display_state.target_position = display_state.ideal_zone_end;
                    } else if (new_mode == DISPLAY_MODE_RUNNING_BACKWARD) {
                        // REQ-DSP-ANIM-02: Animate toward ideal zone start
                        display_state.target_position = display_state.ideal_zone_start;
                    }
                    
                    // Calculate dynamic step delay for constant ~1 second animation
                    // REQ-DSP-ANIM-01 AC-1a, REQ-DSP-ANIM-02 AC-1a
                    if (new_mode == DISPLAY_MODE_RUNNING_FORWARD || 
                        new_mode == DISPLAY_MODE_RUNNING_BACKWARD) {
                        uint32_t steps = (display_state.current_position > display_state.target_position) ?
                                        display_state.current_position - display_state.target_position :
                                        display_state.target_position - display_state.current_position;
                        display_state.step_delay_ms = calculate_step_delay(steps);
                        
                        ESP_LOGD(TAG, "Distance %d mm → LED %d, target %d, steps %lu, delay %lu ms",
                                 measurement->distance_mm, led_index, 
                                 display_state.target_position, steps, display_state.step_delay_ms);
                    }
                    
                    const char *mode_name = (new_mode == DISPLAY_MODE_IDEAL_STEADY) ? "ideal" :
                                           (new_mode == DISPLAY_MODE_RUNNING_FORWARD) ? "too far" : "too close";
                    ESP_LOGD(TAG, "Mode changed to %s zone", mode_name);
                } else {
                    // Same mode, just update measurement position
                    display_state.measurement_position = led_index;
                }
            }
            else if (measurement->distance_mm < config.distance_min_mm)
            {
                // REQ-DSP-ANIM-05: Emergency - too close, blinking pattern
                display_state.mode = DISPLAY_MODE_BLINK_PATTERN;
                if (display_state.last_update == 0) {
                    display_state.last_update = xTaskGetTickCount();
                }
                ESP_LOGD(TAG, "Distance %d mm emergency → blinking pattern", measurement->distance_mm);
            }
            else
            {
                // REQ-DSP-ANIM-04: Too far - steady GREEN on last LED
                display_state.mode = DISPLAY_MODE_STATIC;
                led_clear_all();
                uint16_t led_count = led_get_count();
                led_set_pixel(led_count - 1, LED_COLOR_GREEN);
                led_show();
                ESP_LOGD(TAG, "Distance %d mm too far → LED %d green", measurement->distance_mm, led_count - 1);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Failed to get configuration for display update");
        }
        break;
    }

    case DISTANCE_SENSOR_TIMEOUT:
        // Sensor timeout: All LEDs off
        display_state.mode = DISPLAY_MODE_STATIC;
        led_clear_all();
        led_show();
        ESP_LOGD(TAG, "Sensor timeout → All LEDs off");
        break;

    case DISTANCE_SENSOR_OUT_OF_RANGE:
        // Out of sensor range: Green on last LED (REQ-DSP-ANIM-04)
        display_state.mode = DISPLAY_MODE_STATIC;
        led_clear_all();
        {
            uint16_t led_count = led_get_count();
            led_set_pixel(led_count - 1, LED_COLOR_GREEN);
        }
        led_show();
        ESP_LOGD(TAG, "Sensor out of range → LED %d green", led_get_count() - 1);
        break;

    case DISTANCE_SENSOR_NO_ECHO:
    case DISTANCE_SENSOR_INVALID_READING:
    default:
        // Other errors: Red on first LED
        display_state.mode = DISPLAY_MODE_STATIC;
        led_clear_all();
        led_set_pixel(0, LED_COLOR_RED);
        led_show();
        ESP_LOGD(TAG, "Sensor error → LED 0 red");
        break;
    }
}

/**
 * @brief Main display logic task function
 *
 * Continuously waits for distance measurements and updates LED display with animations.
 * Runs at priority 3 (between distance sensor priority 6 and test priority 2).
 * Uses polling pattern to balance measurement processing and animation updates.
 * 
 * Implements REQ-DSP-IMPL-04: Animation timing independent of measurement rate
 */
static void display_logic_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Display logic task started with animations (Priority: %d, Core: %d)",
             uxTaskPriorityGet(NULL), xPortGetCoreID());

    // Calculate and log dynamic ideal zone boundaries
    uint16_t led_count = led_get_count();
    calculate_ideal_zone(led_count, &display_state.ideal_zone_start, &display_state.ideal_zone_end);
    
    // Get current configuration to log the range
    system_config_t config;
    if (config_get_current(&config) == ESP_OK) {
        ESP_LOGI(TAG, "Distance range: %.1f-%.1fcm → LEDs 0-%d",
                 config.distance_min_mm / 10.0, config.distance_max_mm / 10.0, led_count - 1);
        ESP_LOGI(TAG, "Ideal zone: LEDs %d-%d (dynamic), Animation: ~1s constant duration, Blink: 1Hz",
                 display_state.ideal_zone_start, display_state.ideal_zone_end);
    } else {
        ESP_LOGW(TAG, "Could not get configuration, using defaults");
    }

    distance_measurement_t measurement;
    
    // Animation update interval (50ms = 20fps check rate for smooth animations)
    const TickType_t animation_check_interval = pdMS_TO_TICKS(50);

    while (1)
    {
        // Check for new measurement without blocking
        // This allows animations to continue smoothly
        if (distance_sensor_has_new_measurement())
        {
            // New measurement available, process it
            if (distance_sensor_get_latest(&measurement) == ESP_OK)
            {
                // Update display logic based on new measurement
                update_led_display(&measurement);

                ESP_LOGD(TAG, "Processed distance: %d mm, status: %d, mode: %d",
                         measurement.distance_mm, measurement.status, display_state.mode);
            }
        }
        
        // Update animation state and render (REQ-DSP-IMPL-04)
        // This runs independently of measurement rate for smooth animations
        update_animation_state();
        render_display_state();
        
        // Delay to achieve 20Hz animation check rate
        // Allows other tasks to run and prevents CPU spinning
        vTaskDelay(animation_check_interval);
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
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Display logic task started successfully");
    return ESP_OK;
}

// Legacy functions removed for architectural simplification:
// - display_logic_get_config(): Configuration access now via config_manager API (REQ-CFG-2)
// - display_logic_stop(): Restart-based architecture pattern
// - display_logic_is_running(): Simplified lifecycle management