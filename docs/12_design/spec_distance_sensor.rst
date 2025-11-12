Distance Sensor Design Specification
======================================

This document specifies the design of the HC-SR04 distance sensor component, implementing 
real-time measurement processing with dual-queue architecture and EMA filtering.

**Document Version**: 2.0  
**Last Updated**: 2025-11-12

Overview
--------

The distance sensor component uses a dual-queue architecture to separate ISR-level timestamp 
capture from task-level measurement processing, ensuring real-time performance while providing 
filtered, validated measurements to consumers.

Architecture Specifications
----------------------------

.. spec:: Dual-Queue Real-Time Architecture
   :id: SPEC_SNS_ARCH_1
   :links: REQ_SNS_3, REQ_SNS_4, REQ_SNS_8
   :status: approved
   :tags: sensor, architecture, real-time

   **Design:** Separate ISR and task responsibilities using FreeRTOS queues for communication.

   **Implementation:**

   - ISR captures raw timestamps only, minimal processing
   - Sensor task performs calculations, validation, and smoothing
   - Raw queue (size 2): ISR → Task communication for timestamps
   - Processed queue (size 5): Task → API communication for measurements
   - Queue-based design eliminates shared variables and race conditions

   **Validation:** ISR completes within microseconds, task receives raw timestamps, API 
   consumers get processed measurements.

.. spec:: GPIO Interface Design
   :id: SPEC_SNS_ARCH_2
   :links: REQ_SNS_1, REQ_SNS_2
   :status: approved
   :tags: sensor, gpio, hardware

   **Design:** HC-SR04 ultrasonic sensor interface using ESP32 GPIO.

   **Implementation:**

   - Trigger pin (default GPIO14): Output mode, generates 10µs pulse
   - Echo pin (default GPIO13): Input mode with ``GPIO_INTR_ANYEDGE`` interrupt
   - GPIO configuration during ``distance_sensor_init()`` with validation
   - ISR handler attached to echo pin for edge detection

   **Validation:** GPIO pins configured correctly, ISR service installed, trigger pulse generated.

.. spec:: Interrupt Service Routine Design
   :id: SPEC_SNS_ISR_1
   :links: REQ_SNS_3, REQ_SNS_8
   :status: approved
   :tags: sensor, isr, real-time

   **Design:** IRAM-resident ISR for deterministic timestamp capture.

   **Implementation:**

   - Rising edge: Capture ``echo_start_time`` using ``esp_timer_get_time()``
   - Falling edge: Capture ``echo_end_time`` and queue raw measurement via ``xQueueSendFromISR``
   - Uses ``portYIELD_FROM_ISR`` for task scheduling
   - Marked ``IRAM_ATTR`` for real-time constraints
   - No floating-point, heap allocation, or blocking operations

   **Validation:** ISR execution time < 10µs, timestamps captured accurately, raw queue 
   receives data.

.. spec:: Sensor Task Design
   :id: SPEC_SNS_TASK_1
   :links: REQ_SNS_4, REQ_SNS_6, REQ_SNS_10
   :status: approved
   :tags: sensor, freertos, task

   **Design:** FreeRTOS task for continuous measurement processing.

   **Implementation:**

   - Task priority 5, stack size 4096 bytes, pinned to core 1
   - Measurement loop: trigger pulse → wait for raw queue → calculate distance → validate → smooth → enqueue
   - Configurable measurement interval (default 100ms) with ``vTaskDelay``
   - Timeout handling for missing echo responses (default 30ms)
   - Created by ``distance_sensor_start()``, deleted by ``distance_sensor_stop()``

   **Validation:** Task created successfully, measurement loop operates at configured interval, 
   processes raw data correctly.

Algorithm Specifications
------------------------

.. spec:: Distance Calculation Algorithm
   :id: SPEC_SNS_ALGO_1
   :links: REQ_SNS_11
   :status: approved
   :tags: sensor, algorithm, calibration

   **Design:** Integer arithmetic for temperature-compensated distance calculation.

   **Implementation:**

   - Speed of sound: ``calculate_speed_of_sound_scaled(temperature_c_x10)`` with scaling factor 1,000,000
   - Formula: ``speed = 331300000 + (606 * temperature_c_x10 * 100)``
   - Distance: ``distance_mm = (echo_duration_us * speed_of_sound_scaled) / 2000000``
   - Avoids floating-point operations for embedded performance
   - Temperature input as ``temperature_c_x10`` (200 = 20.0°C)

   **Validation:** Distance calculations accurate within ±1mm for known echo durations and 
   temperatures.

.. spec:: EMA Smoothing Filter Design
   :id: SPEC_SNS_ALGO_2
   :links: REQ_SNS_4
   :status: approved
   :tags: sensor, algorithm, filtering

   **Design:** Exponential Moving Average filter using integer arithmetic.

   **Implementation:**

   - Formula: ``smoothed = (smoothing_factor * new) + ((1000 - smoothing_factor) * previous) / 1000``
   - Smoothing factor range 0-1000 (300 = 30% new, 70% previous)
   - Previous value stored as ``uint16_t previous_smoothed_value_mm``
   - First measurement initializes filter without smoothing
   - Applied only to valid measurements (not out-of-range or timeout)

   **Validation:** Smoothing reduces noise while maintaining responsiveness, factor extremes 
   (0, 1000) work correctly.

API Specifications
------------------

.. spec:: Public API Design
   :id: SPEC_SNS_API_1
   :links: REQ_SNS_5, REQ_SNS_7
   :status: approved
   :tags: sensor, api, interface

   **Design:** Simple blocking and monitoring API for consumers.

   **Implementation:**

   - ``distance_sensor_get_latest()``: Blocking receive from processed queue with ``portMAX_DELAY``
   - ``distance_sensor_has_new_measurement()``: Non-blocking queue status check
   - ``distance_sensor_get_queue_overflows()``: Returns overflow counter for monitoring
   - ``distance_sensor_monitor()``: Health check with overflow logging
   - ``distance_sensor_is_running()``: Task state query

   **Validation:** Blocking API waits for new data, non-blocking API returns immediately, 
   monitoring functions provide accurate metrics.

Error Handling Specifications
------------------------------

.. spec:: Error Handling Design
   :id: SPEC_SNS_ERR_1
   :links: REQ_SNS_12, REQ_SNS_13, REQ_SNS_14
   :status: approved
   :tags: sensor, error-handling, robustness

   **Design:** Comprehensive error detection and recovery.

   **Implementation:**

   - Timeout: No echo within timeout → enqueue ``DISTANCE_SENSOR_TIMEOUT`` with distance 0
   - Out-of-range: Distance < 20mm or > 4000mm → mark ``DISTANCE_SENSOR_OUT_OF_RANGE``, no smoothing
   - Queue overflow: Processed queue full → drop oldest, increment ``queue_overflow_counter``
   - Graceful degradation with status codes in measurement structure

   **Validation:** Error conditions produce correct status codes, overflow policy drops oldest 
   correctly, system continues operation.

Simulator Specification
-----------------------

.. spec:: Distance Sensor Simulator Design
   :id: SPEC_SNS_SIM_1
   :links: REQ_SYS_SIM_1, SPEC_SNS_ARCH_1
   :status: approved
   :tags: sensor, simulator, qemu

   **Design:** Provide a simulator implementation for the distance sensor that implements the 
   full public API declared in ``distance_sensor.h`` while replacing ISR/GPIO timing with a 
   deterministic simulated data producer.

   **Implementation:**

   - API Compatibility: The simulator SHALL implement ``distance_sensor_init()``, ``distance_sensor_start()``, 
     ``distance_sensor_get_latest()`` and all other public functions with identical signatures 
     and return codes
   - Queue Semantics: The simulator MUST produce ``distance_measurement_t`` entries on the 
     processed queue with the same semantics as the hardware task (blocking consumers, identical 
     status codes for out-of-range/timeouts when simulated)
   - Animation Pattern: Deterministic sweep from 5cm to 60cm and back in 1mm steps, advancing 
     once per second
   - Isolation: Simulator implementation SHOULD be in ``distance_sensor_sim.c`` and selected 
     via CMake when ``CONFIG_TARGET_EMULATOR=y`` without modifying headers or higher-level 
     application code

   **Validation:** Simulator build compiles, ``distance_sensor_get_latest()`` receives simulated 
   measurements at ≈1Hz, and status codes match expectations.

   **Example Implementation Pattern:**

   .. code-block:: c

      // Simulated sensor with animated distance sweep
      static void distance_sensor_task(void* pvParameters) {
          static uint16_t sim_distance = 50;  // Start at 5cm
          static int8_t direction = 1;        // 1 = increasing, -1 = decreasing
          
          while(1) {
              // Animate distance: 5cm → 60cm → 5cm (1mm steps)
              sim_distance += direction;
              if (sim_distance >= 600) direction = -1;  // 60cm
              if (sim_distance <= 50)  direction = 1;   // 5cm
              
              distance_measurement_t sim_data = {
                  .distance_mm = sim_distance,
                  .timestamp_us = esp_timer_get_time(),
                  .status = DISTANCE_SENSOR_OK
              };
              
              xQueueSend(processed_measurement_queue, &sim_data, portMAX_DELAY);
              vTaskDelay(pdMS_TO_TICKS(1000));  // 1 second steps
          }
      }
