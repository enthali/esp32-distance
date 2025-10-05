/**
 * @file distance_sensor_sim.c
 * @brief HC-SR04 Ultrasonic Distance Sensor Simulator for QEMU
 *
 * Provides identical API to distance_sensor.c but simulates sensor behavior
 * without requiring hardware. Features animated distance sweep from 5cm to 60cm
 * for testing LED display logic and system integration.
 *
 * SIMULATION BEHAVIOR:
 * ===================
 * - Animated sweep: 5cm → 60cm → 5cm (linear progression)
 * - Step size: 1mm per measurement
 * - Update rate: 1 second intervals (configurable via measurement_interval_ms)
 * - Queue-based architecture identical to hardware implementation
 *
 * API COMPATIBILITY:
 * ==================
 * - Identical function signatures to distance_sensor.c
 * - Same return codes and error handling
 * - Same queue behavior and overflow handling
 * - Same configuration structure support
 *
 * @author ESP32 Distance Project
 * @date 2025
 */

#include "distance_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "distance_sensor_sim";

// Queue sizes - same as hardware implementation
#define PROCESSED_QUEUE_SIZE 5

// Global state
static bool is_initialized = false;
static TaskHandle_t sensor_task_handle = NULL;
static QueueHandle_t processed_measurement_queue = NULL;
static distance_sensor_config_t sensor_config;
static uint32_t queue_overflow_counter = 0;

// Simulation state
static uint16_t sim_distance = 50;  // Start at 5.0cm (50mm)
// use step of 5mm and a direction sign (+1/-1)
static int16_t step_mm = 5;
static int8_t dir_sign = 1;        // +1 = increasing, -1 = decreasing

// Last processed measurement snapshot (non-consuming)
static distance_measurement_t last_processed_measurement = {0};

// Default configuration - same as hardware
static const distance_sensor_config_t default_config = {
    .trigger_pin = GPIO_NUM_14,     // Ignored in simulator
    .echo_pin = GPIO_NUM_13,        // Ignored in simulator
    .measurement_interval_ms = 500, // 500 ms default interval
    .timeout_ms = 30,               // Ignored in simulator
    .temperature_c_x10 = 200,       // Ignored in simulator
    .smoothing_factor = 300         // Ignored in simulator
};

/**
 * @brief Distance sensor simulation task
 *
 * Generates animated distance measurements in a continuous sweep pattern.
 * Maintains identical queue behavior to hardware implementation for API compatibility.
 */
static void distance_sensor_task(void *pvParameters)
{
    ESP_LOGD(TAG, "Distance sensor simulator started (5cm→60cm→5cm sweep, step: %d mm, interval: %lu ms)",
             step_mm, sensor_config.measurement_interval_ms);

    while (1)
    {
        // Animate distance: 5cm (50mm) → 60cm (600mm) → 5cm (50mm)
        // Advance by step_mm in current direction
        if (dir_sign > 0) {
            sim_distance += step_mm;
        } else {
            // avoid underflow on unsigned
            if (sim_distance > (uint16_t)step_mm) {
                sim_distance -= step_mm;
            } else {
                sim_distance = 50;
            }
        }

        // Flip direction at boundaries
        if (sim_distance >= 600) {  // 60.0cm = 600mm
            dir_sign = -1;
        } else if (sim_distance <= 50) {  // 5.0cm = 50mm  
            dir_sign = 1;
        }

        // Create simulated measurement
        distance_measurement_t sim_data = {
            .distance_mm = sim_distance,
            .timestamp_us = esp_timer_get_time(),
            .status = DISTANCE_SENSOR_OK
        };

        // Send to processed queue (handle overflow same as hardware)
        if (xQueueSend(processed_measurement_queue, &sim_data, 0) != pdTRUE)
        {
            // Queue full - remove oldest measurement and add new one
            distance_measurement_t dummy;
            if (xQueueReceive(processed_measurement_queue, &dummy, 0) == pdTRUE)
            {
                xQueueSend(processed_measurement_queue, &sim_data, 0);
                queue_overflow_counter++;
                ESP_LOGW(TAG, "Measurement queue overflow (count: %lu)", queue_overflow_counter);
            }
        }

        // Update last snapshot
        last_processed_measurement = sim_data;

        ESP_LOGD(TAG, "Simulated distance: %.1f cm (%s)", 
             sim_distance / 10.0, 
             dir_sign > 0 ? "increasing" : "decreasing");

        // Sleep for configured interval
        vTaskDelay(pdMS_TO_TICKS(sensor_config.measurement_interval_ms));
    }
}

esp_err_t distance_sensor_init(const distance_sensor_config_t *config)
{
    if (is_initialized)
    {
        ESP_LOGW(TAG, "Distance sensor simulator already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Use provided config or defaults
    if (config != NULL)
    {
        sensor_config = *config;
    }
    else
    {
        sensor_config = default_config;
    }


    // Create processed measurement queue
    processed_measurement_queue = xQueueCreate(PROCESSED_QUEUE_SIZE, sizeof(distance_measurement_t));
    if (processed_measurement_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create processed measurement queue");
        return ESP_ERR_NO_MEM;
    }

    is_initialized = true;
    ESP_LOGD(TAG, "Distance sensor simulator initialized successfully");
    return ESP_OK;
}

esp_err_t distance_sensor_start(void)
{
    if (!is_initialized)
    {
        ESP_LOGE(TAG, "Distance sensor simulator not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (sensor_task_handle != NULL)
    {
        ESP_LOGW(TAG, "Distance sensor simulator task already running");
        return ESP_ERR_INVALID_STATE;
    }

    BaseType_t result = xTaskCreatePinnedToCore(
        distance_sensor_task,
        "distance_sensor_sim",
        4096, // Stack size
        NULL, // Parameters
        5,    // Priority (same as hardware version)
        &sensor_task_handle,
        1     // Core 1
    );

    if (result != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create distance sensor simulator task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Distance sensor simulator started");
    return ESP_OK;
}

esp_err_t distance_sensor_stop(void)
{
    if (!is_initialized)
    {
        ESP_LOGE(TAG, "Distance sensor simulator not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (sensor_task_handle != NULL)
    {
        vTaskDelete(sensor_task_handle);
        sensor_task_handle = NULL;
        ESP_LOGI(TAG, "Distance sensor simulator task stopped");
    }

    if (processed_measurement_queue != NULL)
    {
        vQueueDelete(processed_measurement_queue);
        processed_measurement_queue = NULL;
    }

    queue_overflow_counter = 0;
    is_initialized = false;
    
    ESP_LOGI(TAG, "Distance sensor simulator stopped");
    return ESP_OK;
}

esp_err_t distance_sensor_get_latest(distance_measurement_t *measurement)
{
    if (measurement == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!is_initialized || processed_measurement_queue == NULL)
    {
        ESP_LOGE(TAG, "Distance sensor simulator not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Block until new measurement is available - same behavior as hardware
    return (xQueueReceive(processed_measurement_queue, measurement, portMAX_DELAY) == pdTRUE)
               ? ESP_OK
               : ESP_FAIL;
}

bool distance_sensor_has_new_measurement(void)
{
    if (!is_initialized || processed_measurement_queue == NULL)
    {
        return false;
    }

    return (uxQueueMessagesWaiting(processed_measurement_queue) > 0);
}

uint32_t distance_sensor_get_queue_overflows(void)
{
    return queue_overflow_counter;
}

esp_err_t distance_sensor_monitor(void)
{
    if (!is_initialized)
    {
        ESP_LOGE(TAG, "Distance sensor simulator not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Optional: Add basic health status logging
    if (sensor_task_handle == NULL) {
        ESP_LOGW(TAG, "Distance sensor simulator task not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGD(TAG, "Distance sensor simulator status: running, queue overflow count: %lu", 
             queue_overflow_counter);
    
    return ESP_OK;
}

bool distance_sensor_is_running(void)
{
    return (sensor_task_handle != NULL);
}
