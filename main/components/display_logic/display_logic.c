/**
 * @file display_logic.c
 * @brief LED Distance Visualization business logic with dual-layer rendering.
 *
 * REQUIREMENTS TRACEABILITY:
 * - REQ_DSP_3: Core visualization - dual-layer display with position indicator and zone animations
 * - REQ_DSP_6: Dual-layer rendering architecture (background + position indicator)
 * - REQ_DSP_7: Zone-based display system (5 zones with specific visual patterns)
 * - REQ_DSP_8: Position indicator layer (white LED on top of background)
 * - REQ_DSP_9: Animation layer (directional guidance with timing)
 * - REQ_DSP_10: Ideal zone visualization (consistent reference point)
 *
 * DESIGN TRACEABILITY:
 * - SPEC_DSP_ALGO_1: Distance-to-LED mapping with zone detection
 * - SPEC_DSP_ZONES_1: Zone calculation and boundaries
 * - SPEC_DSP_LAYERS_1: Dual-layer rendering architecture
 * - SPEC_DSP_ANIM_1: Animation patterns and timing
 * 
 * CONFIGURATION:
 * Uses JSON-based config_manager API to read distance parameters:
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
#include "esp_timer.h"

static const char *TAG = "display_logic";

// Task handle
static TaskHandle_t display_task_handle = NULL;

// Configuration parameters (read at initialization per REQ_DSP_2)
static int32_t dist_min_mm = 100;  // Default 100mm = 10cm
static int32_t dist_max_mm = 500;  // Default 500mm = 50cm

// Zone definitions (REQ_DSP_7, SPEC_DSP_ZONES_1)
typedef enum {
    ZONE_0_EMERGENCY = 0,    // Below dist_min_mm
    ZONE_1_TOO_CLOSE = 1,    // 0-20% of LEDs
    ZONE_2_IDEAL = 2,        // 20-40% of LEDs
    ZONE_3_TOO_FAR = 3,      // 40-100% of LEDs
    ZONE_4_OUT_OF_RANGE = 4  // Beyond dist_max_mm
} display_zone_t;

// Animation state for smooth transitions (REQ_DSP_9, SPEC_DSP_ANIM_1)
typedef struct {
    uint32_t animation_step;      // Current animation frame counter
    uint64_t last_update_time;    // Last animation update (microseconds)
    bool blink_state;             // Zone 0 blink state (on/off)
} display_animation_state_t;

static display_animation_state_t animation_state = {0, 0, false};

/**
 * @brief Calculate zone boundaries (REQ_DSP_7, SPEC_DSP_ZONES_1)
 * 
 * Zone boundaries based on LED strip percentage:
 * - Zone 0: Below dist_min_mm (emergency)
 * - Zone 1: 0-20% of LEDs (too close)
 * - Zone 2: 20-40% of LEDs (ideal)
 * - Zone 3: 40-100% of LEDs (too far)
 * - Zone 4: Beyond dist_max_mm (out of range)
 * 
 * @param led_count Number of LEDs in strip
 * @param zone1_end Output: End of Zone 1 (20% boundary)
 * @param zone2_end Output: End of Zone 2 (40% boundary)
 * @param ideal_led Output: Ideal parking position (30% - center of Zone 2)
 */
static void calculate_zone_boundaries(uint16_t led_count, uint16_t *zone1_end, 
                                     uint16_t *zone2_end, uint16_t *ideal_led)
{
    // SPEC_DSP_ZONES_1: Integer arithmetic for zone boundaries
    *zone1_end = led_count * 20 / 100;  // 20% of LEDs
    *zone2_end = led_count * 40 / 100;  // 40% of LEDs
    *ideal_led = led_count * 30 / 100;  // 30% center of ideal zone
}

/**
 * @brief Determine current zone from distance measurement (SPEC_DSP_ALGO_1)
 * 
 * @param measurement Distance measurement
 * @param led_index Output: Calculated LED position (only valid for zones 1-3)
 * @return Current zone
 */
static display_zone_t determine_zone(const distance_measurement_t *measurement, uint16_t *led_index)
{
    // Zone 0: Emergency (below minimum)
    if (measurement->distance_mm < dist_min_mm || measurement->status != DISTANCE_SENSOR_OK)
    {
        return ZONE_0_EMERGENCY;
    }
    
    // Zone 4: Out of range (beyond maximum)
    if (measurement->distance_mm > dist_max_mm)
    {
        return ZONE_4_OUT_OF_RANGE;
    }
    
    // Calculate LED position for zones 1-3 (SPEC_DSP_ALGO_1)
    uint16_t led_count = led_get_count();
    uint32_t range_mm = dist_max_mm - dist_min_mm;
    uint32_t offset_mm = measurement->distance_mm - dist_min_mm;
    
    // Integer math: multiply before divide for precision
    uint32_t calculated_index = (offset_mm * (led_count - 1)) / range_mm;
    
    // Clamp to valid range
    if (calculated_index >= led_count) {
        calculated_index = led_count - 1;
    }
    
    *led_index = (uint16_t)calculated_index;
    
    // Determine zone based on LED position
    uint16_t zone1_end, zone2_end, ideal_led;
    calculate_zone_boundaries(led_count, &zone1_end, &zone2_end, &ideal_led);
    
    if (*led_index < zone1_end) {
        return ZONE_1_TOO_CLOSE;
    } else if (*led_index < zone2_end) {
        return ZONE_2_IDEAL;
    } else {
        return ZONE_3_TOO_FAR;
    }
}

/**
 * @brief Update animation state based on elapsed time (REQ_DSP_9, SPEC_DSP_ANIM_1)
 * 
 * Animation update interval: 100ms (10 Hz frame rate)
 * Zone 0 blink period: 1 Hz (500ms on, 500ms off)
 * 
 * @param zone Current zone
 */
static void update_animation_state(display_zone_t zone)
{
    uint64_t current_time = esp_timer_get_time();
    uint64_t elapsed_us = current_time - animation_state.last_update_time;
    
    // Update animation every 100ms (SPEC_DSP_ANIM_1)
    if (elapsed_us >= 100000)  // 100ms = 100,000 microseconds
    {
        animation_state.animation_step++;
        animation_state.last_update_time = current_time;
        
        // Zone 0 blink logic: toggle every 500ms (5 animation steps)
        if (zone == ZONE_0_EMERGENCY && (animation_state.animation_step % 5) == 0)
        {
            animation_state.blink_state = !animation_state.blink_state;
        }
    }
}

/**
 * @brief Render Zone 0 background: Emergency (REQ_DSP_7 AC-2, SPEC_DSP_ANIM_1)
 * 
 * Pattern: Zone 1 LEDs BLINK RED at 1 Hz, Zones 2 & 3 OFF
 * Rationale: "DANGER! Move backward!" - urgent warning in Zone 1 area
 */
static void render_zone0_background(void)
{
    uint16_t led_count = led_get_count();
    uint16_t zone1_end, zone2_end, ideal_led;
    calculate_zone_boundaries(led_count, &zone1_end, &zone2_end, &ideal_led);
    
    // Zone 1: All LEDs blink red at 1 Hz (500ms on/off)
    if (animation_state.blink_state)
    {
        for (uint16_t i = 0; i < zone1_end; i++)
        {
            led_set_pixel(i, LED_COLOR_RED);
        }
    }
    // Zones 2 & 3: OFF (already cleared)
}

/**
 * @brief Render Zone 1 background: Too Close (REQ_DSP_7 AC-3, SPEC_DSP_ANIM_1)
 * 
 * Pattern: Zone 1 has dim orange (5%) + two adjacent black LEDs chasing from LED 0 to end of Zone 1
 *          Zone 2 shows target in dim red (5%), Zone 3 OFF
 * Rationale: Shows where you are (Zone 1) and where to go (Zone 2 target)
 */
static void render_zone1_background(void)
{
    uint16_t led_count = led_get_count();
    uint16_t zone1_end, zone2_end, ideal_led;
    calculate_zone_boundaries(led_count, &zone1_end, &zone2_end, &ideal_led);
    
    // Zone 1: Dim orange background at 5% brightness (RGB: 255, 165, 0)
    led_color_t orange = led_color_rgb(255, 165, 0);
    led_color_t dim_orange = led_color_brightness(orange, 13);  // 5% of 255 ≈ 13
    for (uint16_t i = 0; i < zone1_end; i++)
    {
        led_set_pixel(i, dim_orange);
    }
    
    // Two adjacent black (off) LEDs in chase pattern FROM LED 0 TOWARD end of Zone 1
    if (zone1_end > 1)  // Need at least 2 LEDs for animation
    {
        uint16_t anim_offset = animation_state.animation_step % zone1_end;
        uint16_t led1_pos = anim_offset;
        uint16_t led2_pos = (anim_offset + 1) % zone1_end;  // Adjacent LED
        
        led_set_pixel(led1_pos, LED_COLOR_OFF);
        led_set_pixel(led2_pos, LED_COLOR_OFF);
    }
    
    // Zone 2: All LEDs at 5% red (shows target parking zone)
    led_color_t dim_red = led_color_brightness(LED_COLOR_RED, 13);  // 5% of 255 ≈ 13
    for (uint16_t i = zone1_end; i < zone2_end; i++)
    {
        led_set_pixel(i, dim_red);
    }
    
    // Zone 3: OFF (already cleared)
}

/**
 * @brief Render Zone 2 background: Ideal (REQ_DSP_7 AC-4, SPEC_DSP_ANIM_1)
 * 
 * Pattern: Zone 1 OFF, Zone 2 solid red (100%), Zone 3 OFF
 * Rationale: "STOP HERE! You are in the ideal parking zone!" - clear visual focus
 */
static void render_zone2_background(void)
{
    uint16_t led_count = led_get_count();
    uint16_t zone1_end, zone2_end, ideal_led;
    calculate_zone_boundaries(led_count, &zone1_end, &zone2_end, &ideal_led);
    
    // Zone 1: OFF (already cleared)
    
    // Zone 2: All LEDs SOLID RED at 100% brightness - "STOP HERE!"
    for (uint16_t i = zone1_end; i < zone2_end; i++)
    {
        led_set_pixel(i, LED_COLOR_RED);
    }
    
    // Zone 3: OFF (already cleared)
}

/**
 * @brief Render Zone 3 background: Too Far (REQ_DSP_7 AC-5, SPEC_DSP_ANIM_1)
 * 
 * Pattern: Zone 1 OFF, Zone 2 shows target in dim green (5%), 
 *          Zone 3 has two adjacent green LEDs (5%) chasing from far end toward start of Zone 3
 * Rationale: Shows where you are (Zone 3) and where to go (Zone 2 target)
 */
static void render_zone3_background(void)
{
    uint16_t led_count = led_get_count();
    uint16_t zone1_end, zone2_end, ideal_led;
    calculate_zone_boundaries(led_count, &zone1_end, &zone2_end, &ideal_led);
    
    // Zone 1: OFF (already cleared)
    
    // Zone 2: All LEDs GREEN at 5% brightness (shows target parking zone)
    led_color_t dim_green = led_color_brightness(LED_COLOR_GREEN, 13);  // 5% of 255 ≈ 13
    for (uint16_t i = zone1_end; i < zone2_end; i++)
    {
        led_set_pixel(i, dim_green);
    }
    
    // Zone 3: Two adjacent green LEDs (5%) in chase pattern FROM far end TOWARD beginning of Zone 3
    uint16_t zone3_size = led_count - zone2_end;
    if (zone3_size > 1)  // Need at least 2 LEDs for animation
    {
        uint16_t anim_offset = animation_state.animation_step % zone3_size;
        uint16_t led1_pos = (led_count - 1) - anim_offset;  // Start from far end, move backward
        uint16_t led2_pos = led1_pos - 1;  // Adjacent LED (one position back)
        
        // Ensure led2_pos stays within Zone 3
        if (led2_pos >= zone2_end)
        {
            led_set_pixel(led1_pos, dim_green);
            led_set_pixel(led2_pos, dim_green);
        }
    }
}

/**
 * @brief Render Zone 4 background: Out of Range (REQ_DSP_7 AC-6, SPEC_DSP_ANIM_1)
 * 
 * Pattern: All zones OFF except last LED at 5% blue
 * Rationale: "Too far, no valid measurement" - minimal informational indicator
 */
static void render_zone4_background(void)
{
    uint16_t led_count = led_get_count();
    
    // Zones 1, 2, 3: OFF (already cleared)
    
    // Last LED: BLUE at 5% brightness - "too far, no valid measurement"
    led_color_t dim_blue = led_color_brightness(LED_COLOR_BLUE, 13);  // 5% of 255 ≈ 13
    led_set_pixel(led_count - 1, dim_blue);
}

/**
 * @brief Update LED display based on distance measurement with dual-layer rendering
 * 
 * DUAL-LAYER RENDERING PIPELINE (REQ_DSP_6, SPEC_DSP_LAYERS_1):
 * 1. Clear all LEDs
 * 2. Render lower layer (zone background pattern)
 * 3. Update animation state
 * 4. Render upper layer (white position indicator)
 * 5. Update physical LEDs with led_show()
 *
 * @param measurement Distance measurement from sensor
 */
static void update_led_display(const distance_measurement_t *measurement)
{    
    // Determine current zone and LED position (SPEC_DSP_ALGO_1)
    uint16_t position_led = 0;
    display_zone_t zone = determine_zone(measurement, &position_led);
    
    // Update animation state (SPEC_DSP_ANIM_1)
    update_animation_state(zone);
    
    // PASS 1: Clear all LEDs (SPEC_DSP_LAYERS_1)
    led_clear_all();
    
    // PASS 2: Render lower layer (zone background) (REQ_DSP_9, SPEC_DSP_LAYERS_1)
    switch (zone)
    {
    case ZONE_0_EMERGENCY:
        render_zone0_background();
        ESP_LOGD(TAG, "Zone 0 (Emergency): distance=%d mm < min=%" PRIi32 " mm", 
                 measurement->distance_mm, dist_min_mm);
        break;
        
    case ZONE_1_TOO_CLOSE:
        render_zone1_background();
        ESP_LOGD(TAG, "Zone 1 (Too Close): LED %d", position_led);
        break;
        
    case ZONE_2_IDEAL:
        render_zone2_background();
        ESP_LOGD(TAG, "Zone 2 (Ideal): LED %d - STOP HERE!", position_led);
        break;
        
    case ZONE_3_TOO_FAR:
        render_zone3_background();
        ESP_LOGD(TAG, "Zone 3 (Too Far): LED %d", position_led);
        break;
        
    case ZONE_4_OUT_OF_RANGE:
        render_zone4_background();
        ESP_LOGD(TAG, "Zone 4 (Out of Range): distance=%d mm > max=%" PRIi32 " mm", 
                 measurement->distance_mm, dist_max_mm);
        break;
    }
    
    // PASS 3: Render upper layer (position indicator) (REQ_DSP_8, SPEC_DSP_LAYERS_1)
    // White LED only visible in zones 1, 2, 3 (valid measurements)
    if (zone >= ZONE_1_TOO_CLOSE && zone <= ZONE_3_TOO_FAR)
    {
        // REQ_DSP_8 AC-1: Position LED is pure white
        // REQ_DSP_8 AC-5: Position LED renders after background (overwrites)
        led_set_pixel(position_led, LED_COLOR_WHITE);
    }
    
    // PASS 4: Update physical LEDs (SPEC_DSP_LAYERS_1)
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

    // Initialize animation state (SPEC_DSP_ANIM_1)
    animation_state.animation_step = 0;
    animation_state.last_update_time = esp_timer_get_time();
    animation_state.blink_state = false;

    // Get LED count for logging
    uint16_t led_count = led_get_count();
    
    // Calculate and log zone boundaries (SPEC_DSP_ZONES_1)
    uint16_t zone1_end, zone2_end, ideal_led;
    calculate_zone_boundaries(led_count, &zone1_end, &zone2_end, &ideal_led);

    ESP_LOGI(TAG, "Display logic initialized successfully");
    ESP_LOGI(TAG, "Distance range: %" PRIi32 "-%" PRIi32 "mm → LEDs 0-%d",
             dist_min_mm, dist_max_mm, led_count - 1);
    ESP_LOGI(TAG, "Zone boundaries: Zone1[0-%d] Zone2[%d-%d] Zone3[%d-%d] Ideal[%d]",
             zone1_end - 1, zone1_end, zone2_end - 1, zone2_end, led_count - 1, ideal_led);

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

// - display_logic_is_running(): Simplified lifecycle management