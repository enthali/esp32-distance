/**
 * @file distance_sensor.c
 * @brief HC-SR04 Ultrasonic Distance Sensor Driver Implementation
 *
 * DUAL-QUEUE REAL-TIME ARCHITECTURE:
 * ==================================
 *
 * This implementation follows real-time system design principles:
 *
 * 1. INTERRUPT SERVICE ROUTINE (ISR):
 *    - Executes in IRAM for deterministic timing
 *    - Only captures timestamps using esp_timer_get_time() (microsecond precision)
 *    - No floating-point calculations (violates real-time constraints)
 *    - Minimal processing to reduce interrupt latency
 *    - Uses xQueueSendFromISR for thread-safe communication
 *
 * 2. SENSOR TASK:
 *    - Triggers measurements at configurable intervals (default: 100ms)
 *    - Blocks on raw_measurement_queue with timeout
 *    - Performs floating-point calculations safely in task context
 *    - Validates measurements and handles errors
 *    - Applies exponential moving average (EMA) filter for smooth readings
 *    - Implements automatic queue overflow handling
 *    - Yields CPU between measurements (vTaskDelay)
 *
 * 3. PUBLIC API:
 *    - Blocking access to measurements (waits for new data)
 *    - Race-condition free (no shared variables)
 *    - Queue-based communication eliminates synchronization issues
 *
 * TIMING CONSIDERATIONS:
 * =====================
 * - HC-SR04 sends 8x 40kHz ultrasonic bursts (takes ~200μs)
 * - Maximum range 400cm = ~23ms echo time
 * - Minimum range 2cm = ~117μs echo time
 * - Temperature affects speed of sound: 331.3 + 0.606 * temp (m/s)
 * - ESP32 timer resolution: 1μs (esp_timer_get_time)
 *
 * QUEUE ARCHITECTURE:
 * ==================
 * Raw Queue (ISR → Task):
 *   - Size: 2 (minimal buffering, ISR priority)
 *   - Contains: Raw timestamps only
 *   - Timeout: 30ms (accommodates max range + margin)
 *
 * Processed Queue (Task → API):
 *   - Size: 5 (buffering for consumer applications)
 *   - Contains: Calculated distances with metadata
 *   - Overflow: Automatic with statistics tracking
 *
 * COMPARED TO VxWorks:
 * ===================
 * - VxWorks msgQ ≈ FreeRTOS xQueue
 * - VxWorks ISR ≈ ESP32 GPIO ISR with IRAM_ATTR
 * - VxWorks timestamps ≈ esp_timer_get_time()
 * - Priority inversion protection via proper task priorities
 */

#include "distance_sensor.h"
#include "config_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_check.h"
#include "nvs.h"
#include <stddef.h> // For offsetof macro

// Debug configuration - comment out to disable detailed debug output
// When defined: Shows struct layout, memory dumps, and detailed EMA filter tracing
// When undefined: Only essential operational logs (initialization, errors, measurements)
// #define DISTANCE_SENSOR_DEBUG

// Measurement logging - comment out to disable frequent measurement logs (every 100ms)
// When defined: Shows each raw/smoothed measurement result
// When undefined: Only initialization, errors, and status messages
// #define DISTANCE_SENSOR_MEASUREMENT_LOGS

static const char *TAG = "distance_sensor";

/**
 * @brief Distance sensor configuration (PRIVATE)
 * 
 * This struct is NOT part of the public API.
 * Configuration is loaded from config_manager during distance_sensor_init().
 */
typedef struct
{
    gpio_num_t trigger_pin;           ///< Trigger pin (default: GPIO14)
    gpio_num_t echo_pin;              ///< Echo pin (default: GPIO13)
    uint32_t measurement_interval_ms; ///< Measurement interval (default: 100ms)
    uint32_t timeout_ms;              ///< Echo timeout (default: 30ms)
    int16_t temperature_c_x10;        ///< Temperature for speed of sound compensation in tenths (default: 200 = 20.0°C)
    uint16_t smoothing_factor;        ///< EMA smoothing factor: 0=heavy smoothing, 1000=no smoothing (default: 300 = 0.3)
} distance_sensor_config_t;

// Configuration
static distance_sensor_config_t sensor_config;

// Task and queue handles
static TaskHandle_t sensor_task_handle = NULL;
static QueueHandle_t raw_measurement_queue = NULL;       // ISR → Sensor Task
static QueueHandle_t processed_measurement_queue = NULL; // Sensor Task → API

// Statistics
static uint32_t queue_overflow_counter = 0;

// Exponential Moving Average (EMA) filter state
static uint16_t previous_smoothed_value_mm = 0;
static bool smoothing_initialized = false;

// ISR state
static volatile uint64_t echo_start_time = 0;
static volatile bool measurement_in_progress = false;

// Queue sizes
#define RAW_QUEUE_SIZE 2       // Minimal ISR buffering
#define PROCESSED_QUEUE_SIZE 5 // API consumer buffering

/*
 * DATA FLOW DIAGRAM:
 * ==================
 *
 * [Sensor Task]                [ISR]                    [API Consumer]
 *      |                        |                           |
 *      | 1. Trigger pulse       |                           |
 *      |--->(GPIO14 HIGH)       |                           |
 *      |                        |                           |
 *      | 2. Wait on queue       | 3. RISING edge            |
 *      |    (blocking)          |    echo_start_time        |
 *      |                        |                           |
 *      |                        | 4. FALLING edge           |
 *      |                        |    raw_data.timestamps    |
 *      |                        |         |                 |
 *      | 5. Calculate distance  |<--------|                 |
 *      |    (floating-point)    |                           |
 *      |    Validate range      |                           |
 *      |    Apply EMA filter    |                           |
 *      |         |              |                           |
 *      |         v              |                           |
 *      | [processed_queue]------|-------------------------->| 6. get_latest()
 *      |                        |                           |    (blocking)
 *      | 7. Sleep 100ms         |                           |
 *      |    (yield CPU)         |                           |
 *      |         |              |                           |
 *      |<--------|              |                           |
 *
 * QUEUE OVERFLOW HANDLING:
 * - Raw queue: Size 2 (ISR critical, minimal buffering)
 * - Processed queue: Size 5 with automatic overflow (remove oldest)
 * - Statistics tracking for monitoring queue health
 */

/**
 * @brief GPIO ISR handler for echo pin - REAL-TIME CRITICAL
 *
 * This function executes in interrupt context and must be IRAM_ATTR for
 * deterministic timing. It only captures timestamps and queues raw data.
 *
 * TIMING REQUIREMENTS:
 * - Must complete within microseconds
 * - No floating-point calculations allowed
 * - No blocking operations
 * - Minimal stack usage
 *
 * HC-SR04 PROTOCOL:
 * - RISING edge: Ultrasonic burst transmitted
 * - FALLING edge: Echo received, calculate pulse width
 * - Pulse width = 2 * distance / speed_of_sound
 */
static void IRAM_ATTR echo_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio_get_level(sensor_config.echo_pin))
    {
        // RISING edge - start timing
        echo_start_time = esp_timer_get_time();
        measurement_in_progress = true;
    }
    else
    {
        // FALLING edge - send raw timestamps
        if (measurement_in_progress)
        {
            distance_raw_measurement_t raw_data = {
                .echo_start_us = echo_start_time,
                .echo_end_us = esp_timer_get_time(),
                .status = DISTANCE_SENSOR_OK};

            // Send to raw queue (non-blocking)
            xQueueSendFromISR(raw_measurement_queue, &raw_data, &xHigherPriorityTaskWoken);
            measurement_in_progress = false;
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Calculate speed of sound based on temperature (integer arithmetic)
 * 
 * Formula: speed = 331.3 + 0.606 * temperature (m/s)
 * Convert to mm/µs: divide by 1000000 and multiply by 1000 = divide by 1000
 * 
 * Integer implementation:
 * - Input: temperature_c_x10 (tenths of Celsius, e.g., 200 = 20.0°C)
 * - Output: speed in mm/µs * 1000000 for precision (later divide by 1000000)
 * 
 * @param temperature_c_x10 Temperature in tenths of Celsius
 * @return uint32_t Speed of sound in (mm/µs * 1000000)
 */
static uint32_t calculate_speed_of_sound_scaled(int16_t temperature_c_x10)
{
    // Convert temperature_c_x10 to actual temperature for calculation
    // temperature_c_x10 = 200 means 20.0°C
    // speed = 331.3 + 0.606 * (temperature_c_x10 / 10.0)
    // speed = 331.3 + (0.606 * temperature_c_x10) / 10
    // speed = 331.3 + (606 * temperature_c_x10) / 10000
    
    // Using integer math with scaling factor 1000000:
    // speed_scaled = (331.3 * 1000000) + (606 * temperature_c_x10 * 1000000) / 10000
    // speed_scaled = 331300000 + (606 * temperature_c_x10 * 100)
    
    int32_t speed_m_per_s_scaled = 331300000 + (606 * temperature_c_x10 * 100);
    
    // Convert from m/s to mm/µs: divide by 1000 (result still scaled by 1000000)
    return (uint32_t)(speed_m_per_s_scaled / 1000);
}

/**
 * @brief Apply exponential moving average (EMA) filter for sensor smoothing (integer arithmetic)
 *
 * The EMA filter provides smooth distance readings while maintaining responsiveness
 * to actual changes. Uses integer arithmetic for embedded performance.
 *
 * Formula: smoothed = (alpha * new_value) + ((1 - alpha) * previous_smoothed)
 * Integer implementation:
 * - smoothing_factor: 0-1000, where 1000=1.0 (no smoothing), 300=0.3 (balanced)
 * - All calculations use integer math with proper scaling
 *
 * Where smoothing_factor controls the balance:
 * - 100: Maximum smoothing (10% new, 90% previous) - very stable but slow
 * - 300: Balanced smoothing (30% new, 70% previous) - good default
 * - 500: Moderate smoothing (50/50 mix)
 * - 1000: No smoothing (100% new, 0% previous) - raw measurements
 *
 * BENEFITS:
 * - Reduces noise from electrical interference and air currents
 * - Maintains responsiveness to real distance changes
 * - Memory efficient (only stores one previous value)
 * - Faster than floating-point arithmetic
 * - No startup delay (works from first measurement)
 *
 * @param new_measurement_mm Raw distance measurement in millimeters
 * @return uint16_t Smoothed distance value in millimeters
 */
static uint16_t apply_ema_filter_int(uint16_t new_measurement_mm)
{
#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "DEBUG: EMA entry - smoothing_factor=%d", sensor_config.smoothing_factor);
#endif

    if (!smoothing_initialized)
    {
        // First measurement - initialize filter with raw value
        previous_smoothed_value_mm = new_measurement_mm;
        smoothing_initialized = true;
#ifdef DISTANCE_SENSOR_DEBUG
        ESP_LOGI(TAG, "EMA filter initialized with first measurement: %d mm (smoothing_factor=%d)",
                 new_measurement_mm, sensor_config.smoothing_factor);
#endif
        return new_measurement_mm;
    }

    // Apply exponential moving average formula with integer arithmetic
    // smoothed = (alpha * new) + ((1 - alpha) * previous)
    // smoothed = (smoothing_factor * new) / 1000 + ((1000 - smoothing_factor) * previous) / 1000
    // Multiply first to avoid precision loss:
    uint32_t smoothed_scaled = (sensor_config.smoothing_factor * new_measurement_mm) +
                              ((1000 - sensor_config.smoothing_factor) * previous_smoothed_value_mm);
    
    uint16_t smoothed = (uint16_t)(smoothed_scaled / 1000);
    
    // Update previous value for next iteration
    previous_smoothed_value_mm = smoothed;

#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "EMA: new=%d mm, previous=%d mm, smoothed=%d mm (factor=%d)", 
             new_measurement_mm, previous_smoothed_value_mm, smoothed, sensor_config.smoothing_factor);
#endif

    return smoothed;
}

/**
 * @brief Distance sensor task - MEASUREMENT PROCESSING
 *
 * This task implements the main measurement loop:
 * 1. Triggers HC-SR04 with 10μs pulse
 * 2. Waits for ISR to provide raw timestamps (blocking with timeout)
 * 3. Calculates distance using temperature-compensated speed of sound
 * 4. Validates measurement range (2-400cm)
 * 5. Queues processed result for API consumption
 * 6. Sleeps to maintain measurement interval and yield CPU
 *
 * PRIORITY DESIGN:
 * - Higher priority than LED test task (5 vs 2)
 * - Lower priority than ISR (hardware interrupt)
 * - Allows precise timing while yielding to other tasks
 *
 * ERROR HANDLING:
 * - Timeout detection for missing echoes
 * - Range validation for impossible readings
 * - Queue overflow management with statistics
 */
static void distance_sensor_task(void *pvParameters)
{
    distance_raw_measurement_t raw_data;
    uint32_t speed_of_sound_scaled = calculate_speed_of_sound_scaled(sensor_config.temperature_c_x10);

    ESP_LOGI(TAG, "Distance sensor task started (interval: %lu ms, timeout: %lu ms)",
             sensor_config.measurement_interval_ms, sensor_config.timeout_ms);

#ifdef DISTANCE_SENSOR_DEBUG
    ESP_LOGI(TAG, "DEBUG: Task started - smoothing_factor=%d", sensor_config.smoothing_factor);
#endif

    while (1)
    {
        // 1. Trigger measurement
        gpio_set_level(sensor_config.trigger_pin, 1);
        esp_rom_delay_us(10); // 10µs trigger pulse
        gpio_set_level(sensor_config.trigger_pin, 0);

        // 2. Wait for ISR result with timeout
        if (xQueueReceive(raw_measurement_queue, &raw_data,
                          pdMS_TO_TICKS(sensor_config.timeout_ms)) == pdPASS)
        {

            // 3. Calculate distance from raw timestamps (integer arithmetic)
            uint64_t echo_duration_us = raw_data.echo_end_us - raw_data.echo_start_us;

            // Distance = (echo_time * speed_of_sound) / 2
            // speed_of_sound_scaled is in (mm/µs * 1000000)
            // Result: distance_mm = (echo_duration_us * speed_of_sound_scaled) / (2 * 1000000)
            uint32_t distance_mm_scaled = (uint32_t)((echo_duration_us * (uint64_t)speed_of_sound_scaled) / 2000000);
            uint16_t distance_mm = (uint16_t)distance_mm_scaled;

            // 4. Validate measurement range and apply smoothing filter
            distance_sensor_error_t status = DISTANCE_SENSOR_OK;
            uint16_t final_distance_mm = distance_mm;

            if (distance_mm < 20 || distance_mm > 4000) // 2.0cm = 20mm, 400.0cm = 4000mm
            {
                status = DISTANCE_SENSOR_OUT_OF_RANGE;
                ESP_LOGW(TAG, "Measurement out of range: %d mm (no smoothing applied)", distance_mm);
                // Don't apply smoothing to invalid measurements - preserve error info
            }
            else
            {
                // Apply EMA filter only to valid measurements
#ifdef DISTANCE_SENSOR_DEBUG
                ESP_LOGI(TAG, "Applying EMA filter to valid measurement: %d mm", distance_mm);
#endif
                final_distance_mm = apply_ema_filter_int(distance_mm);
            }

            // 5. Create processed measurement
            distance_measurement_t processed = {
                .distance_mm = final_distance_mm,
                .timestamp_us = raw_data.echo_end_us,
                .status = status};

            // 6. Send to processed queue (handle overflow)
            if (xQueueSend(processed_measurement_queue, &processed, 0) != pdTRUE)
            {
                // Queue full - remove oldest measurement and add new one
                distance_measurement_t dummy;
                if (xQueueReceive(processed_measurement_queue, &dummy, 0) == pdTRUE)
                {
                    xQueueSend(processed_measurement_queue, &processed, 0);
                    queue_overflow_counter++;
                    ESP_LOGW(TAG, "Measurement queue overflow (count: %lu)", queue_overflow_counter);
                }
            }

#ifdef DISTANCE_SENSOR_MEASUREMENT_LOGS
#ifdef DISTANCE_SENSOR_MEASUREMENT_LOGS
            ESP_LOGI(TAG, "Distance: raw=%d mm, smoothed=%d mm (echo: %llu µs)",
                     distance_mm, final_distance_mm, echo_duration_us);
#endif
#endif
        }
        else
        {
            // Timeout - no echo received
            ESP_LOGW(TAG, "Distance measurement timeout");

            distance_measurement_t timeout_measurement = {
                .distance_mm = 0,
                .timestamp_us = esp_timer_get_time(),
                .status = DISTANCE_SENSOR_TIMEOUT};

            // Send timeout result
            if (xQueueSend(processed_measurement_queue, &timeout_measurement, 0) != pdTRUE)
            {
                distance_measurement_t dummy;
                xQueueReceive(processed_measurement_queue, &dummy, 0);
                xQueueSend(processed_measurement_queue, &timeout_measurement, 0);
                queue_overflow_counter++;
            }
        }

        // 7. Sleep until next measurement
        vTaskDelay(pdMS_TO_TICKS(sensor_config.measurement_interval_ms));
    }
}

esp_err_t distance_sensor_init(gpio_num_t trigger_pin, gpio_num_t echo_pin)
{
    ESP_LOGI(TAG, "Initializing distance sensor (loading config from NVS)...");
    
    // Store hardware pin configuration from parameters
    sensor_config.trigger_pin = trigger_pin;
    sensor_config.echo_pin = echo_pin;
    
    ESP_LOGI(TAG, "Hardware pins: Trigger=GPIO%d, Echo=GPIO%d", trigger_pin, echo_pin);
    
    // Load measurement interval from config_manager (default: 100ms)
    int32_t meas_int_ms = 0;
    esp_err_t ret = config_get_int32("meas_int_ms", &meas_int_ms);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "meas_int_ms not found in NVS, using default: 100ms");
            meas_int_ms = 100;  // Default 100ms interval
        } else {
            ESP_LOGE(TAG, "Failed to read meas_int_ms from config: %s", esp_err_to_name(ret));
            return ret;
        }
    }
    sensor_config.measurement_interval_ms = (uint32_t)meas_int_ms;
    
    // Load sensor timeout from config_manager (default: 30ms)
    int32_t sens_timeout_ms = 0;
    ret = config_get_int32("sens_timeout_ms", &sens_timeout_ms);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "sens_timeout_ms not found in NVS, using default: 30ms");
            sens_timeout_ms = 30;  // Default 30ms timeout
        } else {
            ESP_LOGE(TAG, "Failed to read sens_timeout_ms from config: %s", esp_err_to_name(ret));
            return ret;
        }
    }
    sensor_config.timeout_ms = (uint32_t)sens_timeout_ms;
    
    // Load temperature compensation from config_manager (default: 200 = 20.0°C)
    int32_t temp_c_x10 = 0;
    ret = config_get_int32("temp_c_x10", &temp_c_x10);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "temp_c_x10 not found in NVS, using default: 200 (20.0°C)");
            temp_c_x10 = 200;  // Default to 20°C
        } else {
            ESP_LOGE(TAG, "Failed to read temp_c_x10 from config: %s", esp_err_to_name(ret));
            return ret;
        }
    }
    sensor_config.temperature_c_x10 = (int16_t)temp_c_x10;
    
    // Load smoothing factor from config_manager (default: 300 = balanced smoothing)
    int32_t smooth_factor = 0;
    ret = config_get_int32("smooth_factor", &smooth_factor);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "smooth_factor not found in NVS, using default: 300");
            smooth_factor = 300;  // Default balanced smoothing
        } else {
            ESP_LOGE(TAG, "Failed to read smooth_factor from config: %s", esp_err_to_name(ret));
            return ret;
        }
    }
    sensor_config.smoothing_factor = (uint16_t)smooth_factor;
    
    ESP_LOGI(TAG, "Configuration loaded from NVS:");
    ESP_LOGI(TAG, "  Trigger Pin: GPIO%d (fixed)", sensor_config.trigger_pin);
    ESP_LOGI(TAG, "  Echo Pin: GPIO%d (fixed)", sensor_config.echo_pin);
    ESP_LOGI(TAG, "  Measurement Interval: %lu ms", sensor_config.measurement_interval_ms);
    ESP_LOGI(TAG, "  Sensor Timeout: %lu ms", sensor_config.timeout_ms);
    ESP_LOGI(TAG, "  Temperature: %d (%.1f°C)", sensor_config.temperature_c_x10, sensor_config.temperature_c_x10 / 10.0f);
    ESP_LOGI(TAG, "  Smoothing Factor: %d (%.1f%%)", sensor_config.smoothing_factor, sensor_config.smoothing_factor / 10.0f);
    
    // Initialize EMA filter state
    previous_smoothed_value_mm = 0;
    smoothing_initialized = false;

    // Configure trigger pin as output
    gpio_config_t trigger_conf = {
        .pin_bit_mask = (1ULL << sensor_config.trigger_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_RETURN_ON_ERROR(gpio_config(&trigger_conf), TAG, "Failed to configure trigger pin");

    // Configure echo pin as input with interrupt
    gpio_config_t echo_conf = {
        .pin_bit_mask = (1ULL << sensor_config.echo_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE};
    ESP_RETURN_ON_ERROR(gpio_config(&echo_conf), TAG, "Failed to configure echo pin");

    // Install GPIO ISR service
    ESP_RETURN_ON_ERROR(gpio_install_isr_service(0), TAG, "Failed to install GPIO ISR service");

    // Add ISR handler for echo pin
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(sensor_config.echo_pin, echo_isr_handler, NULL),
                        TAG, "Failed to add GPIO ISR handler");

    // Create queues
    raw_measurement_queue = xQueueCreate(RAW_QUEUE_SIZE, sizeof(distance_raw_measurement_t));
    if (raw_measurement_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create raw measurement queue");
        return ESP_ERR_NO_MEM;
    }

    processed_measurement_queue = xQueueCreate(PROCESSED_QUEUE_SIZE, sizeof(distance_measurement_t));
    if (processed_measurement_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create processed measurement queue");
        vQueueDelete(raw_measurement_queue);
        return ESP_ERR_NO_MEM;
    }

    // Initialize trigger pin to LOW
    gpio_set_level(sensor_config.trigger_pin, 0);

    ESP_LOGI(TAG, "Distance sensor initialized successfully");
    ESP_LOGI(TAG, "EMA filter configured with factor=%d (%d%% new, %d%% previous)",
             sensor_config.smoothing_factor,
             sensor_config.smoothing_factor / 10,
             (1000 - sensor_config.smoothing_factor) / 10);
    return ESP_OK;
}

esp_err_t distance_sensor_start(void)
{
    if (sensor_task_handle != NULL)
    {
        ESP_LOGW(TAG, "Distance sensor task already running");
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        distance_sensor_task,
        "distance_sensor",
        4096, // Stack size
        NULL, // Parameters
        5,    // Priority (higher than LED tests)
        &sensor_task_handle,
        1 // Core 1
    );

    if (result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create distance sensor task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Distance sensor started");
    return ESP_OK;
}

esp_err_t distance_sensor_stop(void)
{
    if (sensor_task_handle == NULL)
    {
        ESP_LOGW(TAG, "Distance sensor task not running");
        return ESP_ERR_INVALID_STATE;
    }

    vTaskDelete(sensor_task_handle);
    sensor_task_handle = NULL;

    ESP_LOGI(TAG, "Distance sensor stopped");
    return ESP_OK;
}

esp_err_t distance_sensor_get_latest(distance_measurement_t *measurement)
{
    if (measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Block until new measurement is available
    return (xQueueReceive(processed_measurement_queue, measurement, portMAX_DELAY) == pdTRUE)
               ? ESP_OK
               : ESP_FAIL;
}

bool distance_sensor_has_new_measurement(void)
{
    return (uxQueueMessagesWaiting(processed_measurement_queue) > 0);
}

uint32_t distance_sensor_get_queue_overflows(void)
{
    return queue_overflow_counter;
}

esp_err_t distance_sensor_monitor(void)
{
    // Check for queue overflows (indicates system overload)
    static uint32_t last_overflow_count = 0;
    uint32_t current_overflows = distance_sensor_get_queue_overflows();
    
    if (current_overflows > last_overflow_count) {
        ESP_LOGW(TAG, "Distance sensor queue overflows: %lu (+%lu new)", 
                 current_overflows, current_overflows - last_overflow_count);
        last_overflow_count = current_overflows;
    }
    
    // Optional: Add basic health status logging
    if (sensor_task_handle == NULL) {
        ESP_LOGW(TAG, "Distance sensor task not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    return ESP_OK;
}

bool distance_sensor_is_running(void)
{
    return (sensor_task_handle != NULL);
}
