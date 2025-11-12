/**
 * @file display_logic.c
 * @brief LED Distance Visualization business logic with dual-layer rendering.
 *
 * Design is documented in `docs/12_design/spec_display.rst` and linked to requirements
 * and design IDs (e.g. REQ_DSP_*, SPEC_DSP_*). Keep this header short; full design
 * rationale lives in the design document for traceability.
 * 
 * CONFIGURATION:
 * Uses new JSON-based config_manager API to read distance parameters:
 * - dist_min_mm: Minimum distance threshold (millimeters)
 * - dist_max_mm: Maximum distance threshold (millimeters)
 * 
 * DESIGN TRACEABILITY:
 * - SPEC_DSP_RENDER_1: Dual-layer rendering architecture
 * - SPEC_DSP_ALGO_4: Animation state machine design
 * 
 * REQUIREMENTS TRACEABILITY:
 * - REQ_DSP_6: Dual-layer LED rendering
 * - REQ_DSP_7: Zone-based guidance system
 * - REQ_DSP_8: Animation system
 */

#include "display_logic.h"
#include "distance_sensor.h"
#include "led_controller.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <inttypes.h>

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

// Configuration parameters (read at initialization per REQ_DSP_2)
static int32_t dist_min_mm = 100;  // Default 100mm = 10cm
static int32_t dist_max_mm = 500;  // Default 500mm = 50cm

/**
 * @brief Zone boundary structure for zone-based guidance
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_ALGO_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7
 */
typedef struct {
    uint16_t zone1_end;  // End of Zone 1 (20% of strip)
    uint16_t zone2_end;  // End of Zone 2 (40% of strip)
} zone_boundaries_t;

/**
 * @brief Animation state structure (stack-allocated)
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_ALGO_4
 * REQUIREMENTS TRACEABILITY: REQ_DSP_8
 */
typedef struct {
    uint32_t frame_counter;      // Total frames rendered since start
    uint32_t last_update_ms;     // Last animation update timestamp
    bool blink_state;            // Zone 0 emergency blink state
} animation_state_t;

/**
 * @brief Create custom RGB color
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1
 */
static led_color_t led_color_rgb_custom(uint8_t r, uint8_t g, uint8_t b)
{
    led_color_t color = {.red = r, .green = g, .blue = b};
    return color;
}

/**
 * @brief Scale color by percentage (integer math)
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7
 * 
 * @param color Original color
 * @param percentage Percentage to scale (0-100)
 * @return Scaled color
 */
static led_color_t led_color_scale(led_color_t color, uint8_t percentage)
{
    led_color_t scaled;
    scaled.red = (color.red * percentage) / 100;
    scaled.green = (color.green * percentage) / 100;
    scaled.blue = (color.blue * percentage) / 100;
    return scaled;
}

/**
 * @brief Calculate zone boundaries based on LED count
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_ALGO_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7 (AC-1: Integer arithmetic)
 * 
 * @param led_count Total number of LEDs
 * @param zones Output structure for zone boundaries
 */
static void calculate_zone_boundaries(uint16_t led_count, zone_boundaries_t* zones)
{
    // Zone 1: 0-20% of strip
    zones->zone1_end = (led_count * 20) / 100;
    
    // Zone 2: 20-40% of strip (ideal parking zone)
    zones->zone2_end = (led_count * 40) / 100;
    
    // Zone 3: 40-100% of strip (implicitly from zone2_end to led_count-1)
    
    ESP_LOGD(TAG, "Zone boundaries: Zone1=[0-%d], Zone2=[%d-%d], Zone3=[%d-%d]",
             zones->zone1_end - 1, zones->zone1_end, zones->zone2_end - 1,
             zones->zone2_end, led_count - 1);
}

/**
 * @brief Get current millisecond timestamp
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_ALGO_4
 */
static uint32_t get_current_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

/**
 * @brief Render Zone 0 - Emergency (distance < dist_min_mm)
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7 (AC-2: 1Hz blink), REQ_DSP_8 (AC-2: 1Hz timing)
 * 
 * All Zone 1 LEDs BLINK RED at 1Hz (500ms on, 500ms off)
 */
static void render_zone_0_emergency(const zone_boundaries_t* zones, animation_state_t* anim)
{
    if (anim->blink_state) {
        // Blink on: Show RED on all Zone 1 LEDs
        led_color_t red = LED_COLOR_RED;
        for (uint16_t i = 0; i < zones->zone1_end; i++) {
            led_set_pixel(i, red);
        }
    }
    // Blink off: LEDs stay black (already cleared)
    
    ESP_LOGD(TAG, "Zone 0 Emergency: Blink=%s", anim->blink_state ? "ON" : "OFF");
}

/**
 * @brief Render Zone 1 - Too Close (0-20% of strip)
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7 (AC-3: Orange background + moving black LEDs)
 * 
 * ORANGE background + 2 BLACK LEDs moving toward ideal zone
 */
static void render_zone_1_too_close(const zone_boundaries_t* zones, animation_state_t* anim)
{
    // Orange background across Zone 1
    led_color_t orange = led_color_rgb_custom(255, 165, 0);
    for (uint16_t i = 0; i < zones->zone1_end; i++) {
        led_set_pixel(i, orange);
    }
    
    // 2 BLACK LEDs moving toward ideal zone (Zone 2)
    if (zones->zone1_end > 2) {
        uint16_t zone_length = zones->zone1_end;
        uint16_t pos = (anim->frame_counter % zone_length);
        
        // Two black LEDs moving from start toward zone 2
        if (pos < zone_length) {
            led_set_pixel(pos, LED_COLOR_OFF);
        }
        if (pos + 1 < zone_length) {
            led_set_pixel(pos + 1, LED_COLOR_OFF);
        }
        
        ESP_LOGD(TAG, "Zone 1 Too Close: Moving indicators at pos %d", pos);
    }
}

/**
 * @brief Render Zone 2 - Ideal (20-40% of strip)
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7 (AC-4: Solid 100% red)
 * 
 * 100% RED → "STOP HERE!" (traffic light concept)
 */
static void render_zone_2_ideal(const zone_boundaries_t* zones)
{
    led_color_t red = LED_COLOR_RED;
    for (uint16_t i = zones->zone1_end; i < zones->zone2_end; i++) {
        led_set_pixel(i, red);
    }
    
    ESP_LOGD(TAG, "Zone 2 Ideal: Solid RED [%d-%d]", zones->zone1_end, zones->zone2_end - 1);
}

/**
 * @brief Render Zone 3 - Too Far (40-100% of strip)
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7 (AC-5: 2 green LEDs at 5% moving)
 * 
 * 2 GREEN LEDs at 5% brightness moving toward ideal zone
 */
static void render_zone_3_too_far(const zone_boundaries_t* zones, animation_state_t* anim, uint16_t led_count)
{
    // 2 GREEN LEDs at 5% brightness moving backward toward Zone 2
    led_color_t green_dim = led_color_scale(LED_COLOR_GREEN, 5);
    
    uint16_t zone_start = zones->zone2_end;
    uint16_t zone_length = led_count - zone_start;
    
    if (zone_length > 2) {
        // Start from far end, move toward Zone 2
        uint16_t cycle_pos = (anim->frame_counter % zone_length);
        uint16_t pos = zone_start + (zone_length - 1 - cycle_pos);
        
        led_set_pixel(pos, green_dim);
        if (pos > zone_start) {
            led_set_pixel(pos - 1, green_dim);
        }
        
        ESP_LOGD(TAG, "Zone 3 Too Far: Moving indicators at pos %d", pos);
    }
}

/**
 * @brief Render Zone 4 - Out of Range (distance > dist_max_mm)
 * 
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1
 * REQUIREMENTS TRACEABILITY: REQ_DSP_7 (AC-6: Last LED blue 5% + ideal zone green 5%)
 * 
 * Last LED 5% BLUE + ideal zone 5% GREEN
 */
static void render_zone_4_out_of_range(uint16_t led_count, const zone_boundaries_t* zones)
{
    // Last LED at 5% BLUE
    led_color_t blue_dim = led_color_scale(LED_COLOR_BLUE, 5);
    led_set_pixel(led_count - 1, blue_dim);
    
    // Ideal zone (Zone 2) at 5% GREEN
    led_color_t green_dim = led_color_scale(LED_COLOR_GREEN, 5);
    for (uint16_t i = zones->zone1_end; i < zones->zone2_end; i++) {
        led_set_pixel(i, green_dim);
    }
    
    ESP_LOGD(TAG, "Zone 4 Out of Range: Blue indicator + ideal zone hint");
}

/**
 * @brief Update LED display based on distance measurement
 *
 * DESIGN TRACEABILITY: SPEC_DSP_RENDER_1 (Dual-layer rendering)
 * REQUIREMENTS TRACEABILITY: REQ_DSP_6 (AC-4: Rendering order)
 * 
 * @param measurement Distance measurement from sensor
 * @param anim Animation state (for timing and frame counter)
 */
static void update_led_display(const distance_measurement_t *measurement, animation_state_t *anim)
{
    uint16_t led_count = led_get_count();
    zone_boundaries_t zones;
    calculate_zone_boundaries(led_count, &zones);
    
    // STEP 1: Clear all LEDs
    led_clear_all();
    
    // STEP 2: Render LOWER LAYER (zone backgrounds and animations)
    switch (measurement->status)
    {
    case DISTANCE_SENSOR_OK:
    {
        if (measurement->distance_mm >= dist_min_mm && measurement->distance_mm <= dist_max_mm)
        {
            // Calculate LED position for zone detection
            uint32_t range_mm = dist_max_mm - dist_min_mm;
            uint32_t offset_mm = measurement->distance_mm - dist_min_mm;
            uint32_t led_index = (offset_mm * (led_count - 1)) / range_mm;
            if (led_index >= led_count) led_index = led_count - 1;
            
            // Determine zone and render appropriate guidance
            if (led_index < zones.zone1_end) {
                // Zone 1: Too close - ORANGE background + moving BLACK LEDs
                render_zone_1_too_close(&zones, anim);
                ESP_LOGD(TAG, "Zone 1: Too close, move backward");
            }
            else if (led_index < zones.zone2_end) {
                // Zone 2: Ideal - 100% RED "STOP HERE!"
                render_zone_2_ideal(&zones);
                ESP_LOGD(TAG, "Zone 2: Ideal parking position");
            }
            else {
                // Zone 3: Too far - Moving GREEN LEDs at 5%
                render_zone_3_too_far(&zones, anim, led_count);
                ESP_LOGD(TAG, "Zone 3: Too far, move forward");
            }
            
            // STEP 3: Render UPPER LAYER (WHITE position indicator)
            // This overwrites the lower layer at the position LED
            led_set_pixel((uint16_t)led_index, LED_COLOR_WHITE);
            ESP_LOGD(TAG, "Position: LED %d (distance %d mm)", (uint16_t)led_index, measurement->distance_mm);
        }
        else if (measurement->distance_mm < dist_min_mm)
        {
            // Zone 0: Emergency - All Zone 1 LEDs BLINK RED at 1Hz
            render_zone_0_emergency(&zones, anim);
            ESP_LOGD(TAG, "Zone 0: Emergency - Too close!");
        }
        else
        {
            // Zone 4: Out of range - Last LED BLUE + ideal zone GREEN hint
            render_zone_4_out_of_range(led_count, &zones);
            ESP_LOGD(TAG, "Zone 4: Out of range - Too far!");
        }
        break;
    }

    case DISTANCE_SENSOR_TIMEOUT:
        // Sensor timeout: All LEDs off (already cleared)
        ESP_LOGD(TAG, "Sensor timeout → All LEDs off");
        break;

    case DISTANCE_SENSOR_OUT_OF_RANGE:
        // Out of sensor range: Zone 4 rendering
        render_zone_4_out_of_range(led_count, &zones);
        ESP_LOGD(TAG, "Sensor out of range → Zone 4 display");
        break;

    case DISTANCE_SENSOR_NO_ECHO:
    case DISTANCE_SENSOR_INVALID_READING:
    default:
        // Other errors: Zone 0 emergency display
        render_zone_0_emergency(&zones, anim);
        ESP_LOGD(TAG, "Sensor error → Emergency display");
        break;
    }

    // STEP 4: Update physical LEDs
    led_show();
}

/**
 * @brief Main display logic task function
 *
 * DESIGN TRACEABILITY: SPEC_DSP_ARCH_1, SPEC_DSP_ALGO_4
 * REQUIREMENTS TRACEABILITY: REQ_DSP_8 (AC-1, AC-2: Animation timing)
 * 
 * Continuously waits for distance measurements and updates LED display.
 * Runs at priority 3 (between distance sensor priority 6 and test priority 2).
 * Blocks on distance_sensor_get_latest() until new measurements arrive.
 * Manages animation timing for zone-based guidance.
 */
static void display_logic_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Display logic task started (Priority: %d, Core: %d)",
             uxTaskPriorityGet(NULL), xPortGetCoreID());

    // Configuration already loaded at initialization (REQ_DSP_2 AC-2)
    ESP_LOGI(TAG, "Distance range: %" PRIi32 "-%" PRIi32 "mm → LEDs 0-%d",
             dist_min_mm, dist_max_mm, led_get_count() - 1);

    // Initialize animation state (stack-allocated per REQ_DSP_8 AC-4)
    animation_state_t anim = {
        .frame_counter = 0,
        .last_update_ms = get_current_ms(),
        .blink_state = false
    };
    
    uint32_t last_blink_toggle_ms = anim.last_update_ms;

    distance_measurement_t measurement;

    while (1)
    {
        // Block until new measurement arrives
        if (distance_sensor_get_latest(&measurement) == ESP_OK)
        {
            uint32_t current_ms = get_current_ms();
            
            // Update animation timing (REQ_DSP_8 AC-1: ~100ms intervals)
            if (current_ms - anim.last_update_ms >= 100) {
                anim.frame_counter++;
                anim.last_update_ms = current_ms;
            }
            
            // Update blink state for Zone 0 (REQ_DSP_8 AC-2: 1Hz = 500ms toggle)
            if (current_ms - last_blink_toggle_ms >= 500) {
                anim.blink_state = !anim.blink_state;
                last_blink_toggle_ms = current_ms;
            }
            
            // Update display with animation state
            update_led_display(&measurement, &anim);

            ESP_LOGD(TAG, "Processed distance: %d mm, status: %d, frame: %d",
                     measurement.distance_mm, measurement.status, anim.frame_counter);
        }
        // No explicit delay - function blocks until next measurement
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