Distance Sensor API
==================

.. apicomponent:: Distance Sensor
   :id: API_COMP_DISTANCE_SENSOR
   :status: implemented
   :header_file: main/components/distance_sensor/distance_sensor.h
   :links: REQ_SNS_1, REQ_SNS_2, REQ_SNS_3, REQ_SNS_4, REQ_SNS_5, REQ_SNS_6, REQ_SNS_7, REQ_SNS_8, REQ_SNS_9, REQ_SNS_10
   
   **Brief Description**

   HC-SR04 ultrasonic distance sensor driver with dual-queue, interrupt-driven architecture for real-time, race-condition-safe measurement processing. Provides blocking and non-blocking APIs, queue overflow statistics, and health monitoring.
   
   Key features:
   
   * Dual-queue architecture: ISR → Task (raw), Task → API (processed)
   * Real-time, race-condition-safe, no floating-point in ISR
   * Temperature compensation, range validation, EMA filtering
   * Blocking and non-blocking measurement APIs
   * Queue overflow statistics and health monitoring
   
   **Architecture:**
   
   - ISR captures timestamps only, sends to raw queue (IRAM_ATTR, real-time safe)
   - Sensor task processes measurements, applies filtering, sends to processed queue
   - API blocks until new data is available, no shared variables
   - Queue overflow handling and statistics
   - Health monitoring for diagnostics

Lifecycle Functions
-------------------

.. apifunction:: distance_sensor_init
   :id: API_FUNC_DISTANCE_SENSOR_INIT
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: esp_err_t distance_sensor_init(gpio_num_t trigger_pin, gpio_num_t echo_pin)
   :returns: ESP_OK on success, ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_ARG, ESP_ERR_*
   :parameters: trigger_pin, echo_pin
   :links: REQ_SNS_1, REQ_SNS_5, REQ_SNS_6, REQ_SNS_9
   
   Initialize the distance sensor, configure GPIO, ISR, and queues. Loads configuration from config_manager.

   **Signature:**

   .. code-block:: c

      esp_err_t distance_sensor_init(gpio_num_t trigger_pin, gpio_num_t echo_pin);

   **Parameters:**

   * ``trigger_pin`` - GPIO pin for HC-SR04 trigger signal
   * ``echo_pin`` - GPIO pin for HC-SR04 echo signal

   **Returns:**

   * ``ESP_OK`` - Initialization successful
   * ``ESP_ERR_INVALID_STATE`` - Config manager not initialized
   * ``ESP_ERR_INVALID_ARG`` - Invalid pin arguments
   * ``ESP_ERR_*`` - Other configuration or hardware errors

   .. note::
      - See requirements: REQ_SNS_1, REQ_SNS_5, REQ_SNS_6, REQ_SNS_9
      - Dual-queue, race-condition-safe, config_manager integration

.. apifunction:: distance_sensor_start
   :id: API_FUNC_DISTANCE_SENSOR_START
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: esp_err_t distance_sensor_start(void)
   :returns: ESP_OK on success
   :parameters: None
   :links: REQ_SNS_2, REQ_SNS_3, REQ_SNS_4, REQ_SNS_8
   
   Start distance measurements, create measurement task, trigger sensors, process results.

   **Signature:**

   .. code-block:: c

      esp_err_t distance_sensor_start(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Task started successfully

   .. note::
      - See requirements: REQ_SNS_2, REQ_SNS_3, REQ_SNS_4, REQ_SNS_8
      - Dual-queue, real-time, blocking API

.. apifunction:: distance_sensor_stop
   :id: API_FUNC_DISTANCE_SENSOR_STOP
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: esp_err_t distance_sensor_stop(void)
   :returns: ESP_OK on success
   :parameters: None
   :links: REQ_SNS_8
   
   Stop distance measurements and measurement task.

   **Signature:**

   .. code-block:: c

      esp_err_t distance_sensor_stop(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Task stopped successfully

   .. note::
      - See requirements: REQ_SNS_8
      - API blocking/read semantics

Data Access Functions
---------------------

.. apifunction:: distance_sensor_get_latest
   :id: API_FUNC_DISTANCE_SENSOR_GET_LATEST
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: esp_err_t distance_sensor_get_latest(distance_measurement_t *measurement)
   :returns: ESP_OK on success, ESP_ERR_INVALID_ARG
   :parameters: measurement
   :links: REQ_SNS_8, REQ_SNS_9
   
   Get the latest distance measurement (blocking until new data is available).

   **Signature:**

   .. code-block:: c

      esp_err_t distance_sensor_get_latest(distance_measurement_t *measurement);

   **Parameters:**

   * ``measurement`` - Pointer to store the measurement result

   **Returns:**

   * ``ESP_OK`` - Measurement available
   * ``ESP_ERR_INVALID_ARG`` - Null pointer

   .. note::
      - See requirements: REQ_SNS_8, REQ_SNS_9
      - Blocking, race-condition-safe

.. apifunction:: distance_sensor_has_new_measurement
   :id: API_FUNC_DISTANCE_SENSOR_HAS_NEW_MEASUREMENT
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: bool distance_sensor_has_new_measurement(void)
   :returns: true if new measurement available, false otherwise
   :parameters: None
   :links: REQ_SNS_8
   
   Check if a new measurement is available (non-blocking).

   **Signature:**

   .. code-block:: c

      bool distance_sensor_has_new_measurement(void);

   **Parameters:**

   * None

   **Returns:**

   * ``true`` - New measurement available
   * ``false`` - No new measurement

   .. note::
      - See requirements: REQ_SNS_8
      - Non-blocking API

Statistics & Monitoring Functions
---------------------------------

.. apifunction:: distance_sensor_get_queue_overflows
   :id: API_FUNC_DISTANCE_SENSOR_GET_QUEUE_OVERFLOWS
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: uint32_t distance_sensor_get_queue_overflows(void)
   :returns: Number of measurements discarded due to queue overflow
   :parameters: None
   :links: REQ_SNS_6, REQ_SNS_10
   
   Get the number of queue overflows (discarded measurements).

   **Signature:**

   .. code-block:: c

      uint32_t distance_sensor_get_queue_overflows(void);

   **Parameters:**

   * None

   **Returns:**

   * ``uint32_t`` - Number of discarded measurements

   .. note::
      - See requirements: REQ_SNS_6, REQ_SNS_10
      - Diagnostics/statistics

.. apifunction:: distance_sensor_monitor
   :id: API_FUNC_DISTANCE_SENSOR_MONITOR
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: esp_err_t distance_sensor_monitor(void)
   :returns: ESP_OK on success
   :parameters: None
   :links: REQ_SNS_7, REQ_SNS_10
   
   Perform lightweight sensor health monitoring (queue overflow, health status).

   **Signature:**

   .. code-block:: c

      esp_err_t distance_sensor_monitor(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Monitoring successful

   .. note::
      - See requirements: REQ_SNS_7, REQ_SNS_10
      - Health monitoring, diagnostics

.. apifunction:: distance_sensor_is_running
   :id: API_FUNC_DISTANCE_SENSOR_IS_RUNNING
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :api_signature: bool distance_sensor_is_running(void)
   :returns: true if sensor task is running, false otherwise
   :parameters: None
   :links: REQ_SNS_8
   
   Check if the sensor task is running.

   **Signature:**

   .. code-block:: c

      bool distance_sensor_is_running(void);

   **Parameters:**

   * None

   **Returns:**

   * ``true`` - Sensor task running
   * ``false`` - Not running

   .. note::
      - See requirements: REQ_SNS_8
      - API blocking/read semantics

Data Types
----------

.. apistruct:: distance_sensor_error_t
   :id: API_STRUCT_DISTANCE_SENSOR_ERROR
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :links: REQ_SNS_4
   
   Error codes for distance sensor measurements.

   **Definition:**

   .. code-block:: c

      typedef enum {
          DISTANCE_SENSOR_OK = 0,
          DISTANCE_SENSOR_TIMEOUT,
          DISTANCE_SENSOR_OUT_OF_RANGE,
          DISTANCE_SENSOR_NO_ECHO,
          DISTANCE_SENSOR_INVALID_READING
      } distance_sensor_error_t;

   **Values:**

   * ``DISTANCE_SENSOR_OK`` - Measurement successful
   * ``DISTANCE_SENSOR_TIMEOUT`` - Echo not received in time
   * ``DISTANCE_SENSOR_OUT_OF_RANGE`` - Measurement out of valid range
   * ``DISTANCE_SENSOR_NO_ECHO`` - No echo detected
   * ``DISTANCE_SENSOR_INVALID_READING`` - Invalid or corrupted reading

.. apistruct:: distance_raw_measurement_t
   :id: API_STRUCT_DISTANCE_SENSOR_RAW_MEASUREMENT
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :links: REQ_SNS_3
   
   Raw measurement data from ISR (timestamps only).

   **Definition:**

   .. code-block:: c

      typedef struct {
          uint64_t echo_start_us;
          uint64_t echo_end_us;
          distance_sensor_error_t status;
      } distance_raw_measurement_t;

   **Fields:**

   * ``echo_start_us`` - Echo start timestamp (microseconds)
   * ``echo_end_us`` - Echo end timestamp (microseconds)
   * ``status`` - Measurement status (see distance_sensor_error_t)

.. apistruct:: distance_measurement_t
   :id: API_STRUCT_DISTANCE_SENSOR_MEASUREMENT
   :status: implemented
   :component: API_COMP_DISTANCE_SENSOR
   :links: REQ_SNS_4
   
   Processed distance measurement.

   **Definition:**

   .. code-block:: c

      typedef struct {
          uint16_t distance_mm;
          uint64_t timestamp_us;
          distance_sensor_error_t status;
      } distance_measurement_t;

   **Fields:**

   * ``distance_mm`` - Calculated distance in millimeters (0-65535mm)
   * ``timestamp_us`` - Measurement timestamp (microseconds)
   * ``status`` - Measurement status (see distance_sensor_error_t)

