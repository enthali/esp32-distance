Distance Sensor Requirements
=============================

This document specifies requirements for the HC-SR04 ultrasonic distance sensor component.

**Document Version**: 2.0  
**Last Updated**: 2025-11-12

Overview
--------

The distance sensor component provides precise distance measurement with real-time processing,
noise reduction via exponential moving average filtering, and robust error handling.

Functional Requirements
-----------------------

.. req:: Component Initialization
   :id: REQ_SNS_1
   :links: REQ_SYS_1
   :status: approved
   :priority: mandatory
   :tags: sensor, initialization, gpio

   **Description:** The distance sensor component SHALL provide initialization via 
   ``distance_sensor_init(const distance_sensor_config_t *config)`` that configures GPIO pins, 
   installs ISR service, creates internal queues, and establishes default configuration when 
   no config is provided.

   **Rationale:** Deterministic startup sequence is required for reliable sensor operation and 
   system integration.

   **Acceptance Criteria:**

   - AC-1: ``distance_sensor_init(NULL)`` SHALL return ``ESP_OK`` and use default configuration values
   - AC-2: Trigger pin SHALL be configured as GPIO output with no pull-up/pull-down
   - AC-3: Echo pin SHALL be configured as GPIO input with ``GPIO_INTR_ANYEDGE`` interrupt
   - AC-4: GPIO ISR service SHALL be installed successfully
   - AC-5: Raw measurement queue (size 2) and processed measurement queue (size 5) SHALL be created
   - AC-6: Function SHALL return appropriate ``esp_err_t`` codes for error conditions

   **Verification:** Code review of GPIO configuration and integration test validating successful 
   initialization with NULL and custom configurations.

.. req:: Trigger Pulse Generation
   :id: REQ_SNS_2
   :links: REQ_SNS_1
   :status: approved
   :priority: mandatory
   :tags: sensor, trigger, timing

   **Description:** The sensor task SHALL generate a 10µs trigger pulse on the configured trigger 
   GPIO pin for each measurement cycle to initiate HC-SR04 ultrasonic measurement.

   **Rationale:** HC-SR04 datasheet specifies 10µs trigger pulse requirement for reliable operation.

   **Acceptance Criteria:**

   - AC-1: Trigger pulse duration SHALL be 10µs ±1µs
   - AC-2: Trigger pulse SHALL transition from LOW to HIGH to LOW
   - AC-3: Pulse generation SHALL occur at the configured measurement interval
   - AC-4: Pulse generation SHALL use ``esp_rom_delay_us()`` for precise timing

   **Verification:** Logic analyzer capture of trigger pin or instrumented test measuring pulse 
   width and timing accuracy.

.. req:: Real-Time Timestamp Capture
   :id: REQ_SNS_3
   :links: REQ_SNS_1, REQ_SNS_2
   :status: approved
   :priority: mandatory
   :tags: sensor, isr, timing, real-time

   **Description:** The GPIO ISR SHALL capture rising and falling edge timestamps using 
   ``esp_timer_get_time()`` with microsecond precision and queue raw timestamp data to the 
   sensor task without blocking operations.

   **Rationale:** Accurate distance calculation requires precise echo timing; ISR must remain 
   minimal for real-time system stability.

   **Acceptance Criteria:**

   - AC-1: ISR SHALL be marked ``IRAM_ATTR`` for deterministic execution
   - AC-2: Rising edge SHALL capture start timestamp to volatile variable
   - AC-3: Falling edge SHALL capture end timestamp and queue ``distance_raw_measurement_t`` via ``xQueueSendFromISR``
   - AC-4: ISR SHALL NOT perform floating-point operations, heap allocation, or blocking calls
   - AC-5: ISR SHALL use ``portYIELD_FROM_ISR`` when higher priority task awakened
   - AC-6: Raw timestamp data SHALL be queued successfully or measurement marked invalid

   **Verification:** Code review for ISR constraints and timing analysis showing ISR execution 
   time < 10µs.

.. req:: Measurement Processing
   :id: REQ_SNS_4
   :links: REQ_SNS_3
   :status: approved
   :priority: mandatory
   :tags: sensor, processing, filtering

   **Description:** The sensor task SHALL calculate distance in millimeters using integer 
   arithmetic with temperature compensation, validate measurement range (20mm-4000mm), apply 
   exponential moving average smoothing for valid measurements, and enqueue processed results.

   **Rationale:** Separation of concerns keeps ISR minimal while providing filtered, validated 
   measurements to consumers.

   **Acceptance Criteria:**

   - AC-1: Distance calculation SHALL use integer arithmetic with temperature compensation
   - AC-2: Measurements below 20mm or above 4000mm SHALL be marked ``DISTANCE_SENSOR_OUT_OF_RANGE``
   - AC-3: Valid measurements SHALL be processed through EMA filter based on ``smoothing_factor``
   - AC-4: Out-of-range measurements SHALL NOT update EMA filter state
   - AC-5: Processed measurements SHALL include ``distance_mm``, ``timestamp_us``, and ``status`` fields
   - AC-6: Processed measurements SHALL be queued to processed measurement queue

   **Verification:** Unit tests with synthetic timestamps validating distance calculations, range 
   validation, and EMA filter behavior.

.. req:: Blocking API Access
   :id: REQ_SNS_5
   :links: REQ_SNS_4
   :status: approved
   :priority: mandatory
   :tags: sensor, api, synchronization

   **Description:** The component SHALL provide ``distance_sensor_get_latest(distance_measurement_t *measurement)`` 
   that blocks until the next processed measurement is available, guaranteeing fresh data to consumers.

   **Rationale:** Simplified consumer API that eliminates polling overhead and ensures data freshness.

   **Acceptance Criteria:**

   - AC-1: Function SHALL block with ``portMAX_DELAY`` until processed measurement available
   - AC-2: Function SHALL return ``ESP_OK`` when measurement successfully retrieved
   - AC-3: Function SHALL return ``ESP_ERR_INVALID_ARG`` when measurement pointer is NULL
   - AC-4: Function SHALL copy complete measurement structure to caller's buffer
   - AC-5: Function SHALL be thread-safe for multiple consumers

   **Verification:** Integration test with consumer task blocking on API and validating received 
   measurements.

.. req:: Task Lifecycle Management
   :id: REQ_SNS_6
   :links: REQ_SNS_1
   :status: approved
   :priority: mandatory
   :tags: sensor, lifecycle, freertos

   **Description:** The component SHALL provide ``distance_sensor_start()`` and ``distance_sensor_stop()`` 
   functions to control sensor task creation and deletion with proper state management.

   **Rationale:** Controlled startup and graceful shutdown enable system power management and error 
   recovery.

   **Acceptance Criteria:**

   - AC-1: ``distance_sensor_start()`` SHALL create sensor task pinned to core 1 with priority 5
   - AC-2: Task SHALL have stack size of 4096 bytes and name "distance_sensor"
   - AC-3: ``distance_sensor_stop()`` SHALL delete sensor task and set handle to NULL
   - AC-4: Functions SHALL return appropriate error codes for invalid state transitions
   - AC-5: ``distance_sensor_is_running()`` SHALL reflect actual task state

   **Verification:** Unit tests validating task creation, deletion, and state management across 
   start/stop cycles.

.. req:: Health Monitoring
   :id: REQ_SNS_7
   :links: REQ_SNS_4
   :status: approved
   :priority: important
   :tags: sensor, monitoring, diagnostics

   **Description:** The component SHALL provide monitoring capabilities including queue overflow 
   metrics and health status reporting for operational visibility.

   **Rationale:** System operators need visibility into sensor health and potential overload conditions.

   **Acceptance Criteria:**

   - AC-1: ``distance_sensor_get_queue_overflows()`` SHALL return monotonically increasing counter
   - AC-2: ``distance_sensor_monitor()`` SHALL log new overflow events with counts
   - AC-3: ``distance_sensor_has_new_measurement()`` SHALL return queue status without blocking
   - AC-4: Monitoring functions SHALL be callable regardless of task state
   - AC-5: Overflow counter SHALL increment only when processed queue full and oldest dropped

   **Verification:** Stress test forcing queue overflows and validating counter behavior and logging 
   output.

Non-Functional Requirements
---------------------------

.. req:: Real-Time ISR Constraints
   :id: REQ_SNS_8
   :links: REQ_SNS_3
   :status: approved
   :priority: critical
   :tags: sensor, performance, real-time, isr

   **Description:** The GPIO ISR SHALL execute within microsecond timeframes and comply with 
   real-time system constraints by avoiding floating-point operations, heap allocations, and 
   blocking calls.

   **Rationale:** ESP32 real-time stability requires deterministic ISR behavior to prevent system 
   jitter and timing violations.

   **Acceptance Criteria:**

   - AC-1: ISR execution time SHALL be < 10µs in worst case
   - AC-2: ISR SHALL be marked ``IRAM_ATTR`` and reside in instruction RAM
   - AC-3: ISR SHALL NOT use floating-point arithmetic
   - AC-4: ISR SHALL NOT call ``malloc``, ``free``, or heap allocation functions
   - AC-5: ISR SHALL NOT call blocking FreeRTOS functions except ``xQueueSendFromISR``
   - AC-6: ISR stack usage SHALL be < 512 bytes

   **Verification:** Static analysis for forbidden operations and timing measurement under maximum 
   load conditions.

.. req:: Memory Resource Limits
   :id: REQ_SNS_9
   :links: REQ_SNS_1
   :status: approved
   :priority: important
   :tags: sensor, memory, resource

   **Description:** The distance sensor component SHALL operate within fixed memory limits using 
   small queue sizes and bounded stack allocation to fit ESP32 memory constraints.

   **Rationale:** ESP32 has limited RAM; component must be memory-efficient while maintaining 
   functionality.

   **Acceptance Criteria:**

   - AC-1: Raw measurement queue size SHALL be exactly 2 elements
   - AC-2: Processed measurement queue size SHALL be exactly 5 elements
   - AC-3: Sensor task stack size SHALL be 4096 bytes
   - AC-4: Component SHALL use no dynamic memory allocation after initialization
   - AC-5: Static memory usage SHALL be < 1KB for variables and state
   - AC-6: Queue element sizes SHALL be minimal for required data

   **Verification:** Memory usage analysis and heap monitoring during component operation.

.. req:: Timing and Performance
   :id: REQ_SNS_10
   :links: REQ_SNS_2, REQ_SNS_4
   :status: approved
   :priority: important
   :tags: sensor, performance, timing

   **Description:** The sensor component SHALL operate at configurable measurement intervals with 
   deterministic timing characteristics and appropriate timeout handling.

   **Rationale:** Predictable measurement timing enables system coordination and proper sensor 
   operation.

   **Acceptance Criteria:**

   - AC-1: Default measurement interval SHALL be 100ms (10Hz rate)
   - AC-2: Measurement interval SHALL be configurable via ``measurement_interval_ms``
   - AC-3: Default echo timeout SHALL be 30ms to accommodate maximum range
   - AC-4: Echo timeout SHALL be configurable via ``timeout_ms``
   - AC-5: Task SHALL yield CPU between measurements using ``vTaskDelay``
   - AC-6: Measurement timing SHALL have ±5ms accuracy at 100ms interval

   **Verification:** Timing analysis with oscilloscope or logic analyzer measuring actual intervals 
   and jitter.

.. req:: Accuracy and Calibration
   :id: REQ_SNS_11
   :links: REQ_SNS_4
   :status: approved
   :priority: mandatory
   :tags: sensor, accuracy, calibration

   **Description:** Distance calculations SHALL use integer arithmetic with temperature compensation 
   to achieve accurate measurements across operational temperature range.

   **Rationale:** Measurement accuracy is critical for application functionality; temperature affects 
   sound speed significantly.

   **Acceptance Criteria:**

   - AC-1: Speed of sound calculation SHALL use formula: 331.3 + 0.606 * temperature (m/s)
   - AC-2: Temperature input SHALL be ``temperature_c_x10`` format (200 = 20.0°C)
   - AC-3: Integer arithmetic SHALL use scaling factor 1,000,000 for precision
   - AC-4: Distance accuracy SHALL be ±2mm for distances 20mm-400cm at 20°C
   - AC-5: EMA smoothing factor SHALL be configurable 0-1000 (1000 = no smoothing)
   - AC-6: First measurement SHALL initialize EMA filter without smoothing

   **Verification:** Calibration tests with known distances at various temperatures and EMA filter 
   validation.

Error Handling Requirements
---------------------------

.. req:: Timeout Error Handling
   :id: REQ_SNS_12
   :links: REQ_SNS_3, REQ_SNS_4
   :status: approved
   :priority: mandatory
   :tags: sensor, error-handling, timeout

   **Description:** The system SHALL handle missing echo signals by generating timeout measurements 
   with appropriate status codes when no echo is received within the configured timeout period.

   **Rationale:** Missing echoes occur due to signal absorption or reflection angles; system must 
   continue operation.

   **Acceptance Criteria:**

   - AC-1: Timeout condition SHALL be detected when no echo received within ``timeout_ms``
   - AC-2: Timeout measurement SHALL have status ``DISTANCE_SENSOR_TIMEOUT``
   - AC-3: Timeout measurement SHALL have distance value 0
   - AC-4: Timeout measurement SHALL include valid timestamp
   - AC-5: System SHALL continue normal operation after timeout
   - AC-6: Timeout events SHALL be logged at warning level

   **Verification:** Test with echo pin disconnected or sensor obstruction causing timeouts.

.. req:: Range Validation
   :id: REQ_SNS_13
   :links: REQ_SNS_4
   :status: approved
   :priority: mandatory
   :tags: sensor, error-handling, validation

   **Description:** The system SHALL validate measurement range and flag out-of-range values without 
   applying smoothing to preserve error information.

   **Rationale:** Out-of-range measurements indicate sensor issues or environmental conditions 
   requiring user attention.

   **Acceptance Criteria:**

   - AC-1: Measurements < 20mm SHALL be marked ``DISTANCE_SENSOR_OUT_OF_RANGE``
   - AC-2: Measurements > 4000mm SHALL be marked ``DISTANCE_SENSOR_OUT_OF_RANGE``
   - AC-3: Out-of-range measurements SHALL NOT update EMA filter state
   - AC-4: Out-of-range measurements SHALL preserve raw distance value
   - AC-5: Range validation SHALL occur before smoothing filter
   - AC-6: Out-of-range events SHALL be logged at warning level

   **Verification:** Unit tests with synthetic timestamps producing out-of-range distances and 
   validation of status codes.

.. req:: Queue Overflow Management
   :id: REQ_SNS_14
   :links: REQ_SNS_4, REQ_SNS_7
   :status: approved
   :priority: important
   :tags: sensor, error-handling, queue

   **Description:** The system SHALL implement drop-oldest policy for processed queue overflow with 
   statistics tracking to handle consumer backpressure without blocking sensor operation.

   **Rationale:** Slow consumers should not disrupt sensor measurement timing; overflow indicates 
   system load issues.

   **Acceptance Criteria:**

   - AC-1: Processed queue full condition SHALL trigger drop-oldest behavior
   - AC-2: Oldest measurement SHALL be removed before adding new measurement
   - AC-3: Queue overflow counter SHALL increment on each overflow event
   - AC-4: Overflow events SHALL be logged with cumulative count
   - AC-5: Sensor task SHALL never block on processed queue operations
   - AC-6: Raw queue overflow SHALL be prevented by proper task priority

   **Verification:** Load testing with slow consumer to force overflows and validate drop-oldest 
   behavior and statistics.
