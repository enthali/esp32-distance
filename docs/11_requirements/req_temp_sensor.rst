Temperature Sensor Requirements
================================

This document specifies requirements for the DS18B20 Dallas 1-Wire temperature sensor component.

**Document Version**: 1.0
**Last Updated**: 2026-03-17

Overview
--------

The temperature sensor component provides ambient temperature measurement via a Dallas DS18B20
1-Wire digital temperature sensor. It serves two purposes: supplying live temperature data to
the distance sensor for speed-of-sound compensation, and providing ambient temperature
information to the display and web interface.

Functional Requirements
-----------------------

.. req:: DS18B20 Component Initialization
   :id: REQ_TEMP_1
   :status: approved
   :priority: mandatory
   :links: REQ_SYS_TEMP_1
   :tags: temperature, sensor, initialization, gpio

   **Description:** The temperature sensor component SHALL provide initialization via
   ``temp_sensor_init(const temp_sensor_config_t *config)`` that configures the GPIO data pin,
   initialises internal state, and uses default configuration when no config is provided.

   **Rationale:** Deterministic startup sequence required for reliable sensor operation and
   system integration.

   **Acceptance Criteria:**

   - AC-1: ``temp_sensor_init(NULL)`` SHALL return ``ESP_OK`` and use GPIO4 as default data pin
   - AC-2: Data pin SHALL be configured as open-drain output with internal pull-up (external 4.7kΩ pull-up recommended in hardware design)
   - AC-3: Function SHALL return appropriate ``esp_err_t`` codes for error conditions
   - AC-4: Component SHALL support a single DS18B20 sensor

   **Verification:** Unit test validating successful init with NULL and explicit config.


.. req:: Dallas 1-Wire Protocol Communication
   :id: REQ_TEMP_2
   :status: approved
   :priority: mandatory
   :links: REQ_TEMP_1
   :tags: temperature, sensor, onewire, protocol

   **Description:** The component SHALL implement Dallas 1-Wire bit-banging protocol over a single
   GPIO data pin to communicate with the DS18B20 sensor.

   **Rationale:** DS18B20 exclusively uses 1-Wire protocol. Bit-banging avoids hardware peripheral
   dependencies and works on any GPIO.

   **Acceptance Criteria:**

   - AC-1: Reset pulse SHALL conform to DS18B20 datasheet (480µs LOW, release, 60µs sample window)
   - AC-2: Write-1 and Write-0 slots SHALL meet DS18B20 timing requirements
   - AC-3: Read slots SHALL capture valid data bits within DS18B20 timing window
   - AC-4: 12-bit resolution SHALL be used for ±0.0625°C measurement precision
   - AC-5: Timing-critical sections SHALL disable interrupts to prevent jitter

   **Verification:** Logic analyser trace confirming protocol timing; integration test with real hardware.


.. req:: Periodic Temperature Measurement Task
   :id: REQ_TEMP_3
   :status: approved
   :priority: mandatory
   :links: REQ_TEMP_2
   :tags: temperature, sensor, measurement, freertos

   **Description:** The component SHALL run a FreeRTOS task that periodically initiates DS18B20
   temperature conversion and retrieves the result.

   **Rationale:** DS18B20 conversion takes ≥750ms at 12-bit resolution. A dedicated task allows
   correct sequencing (initiate → wait → read) without blocking other components.

   **Acceptance Criteria:**

   - AC-1: Default measurement interval SHALL be 2000ms (0.5 Hz)
   - AC-2: Interval SHALL be configurable via ``temp_interval_ms`` config key
   - AC-3: Task SHALL wait at least 750ms after conversion start before reading result
   - AC-4: Task SHALL use ``vTaskDelay`` during conversion wait (no busy-wait)
   - AC-5: Task stack size SHALL be ≤ 2048 bytes

   **Verification:** Integration test measuring actual interval timing; stack high-water mark monitoring.


.. req:: Temperature Data API
   :id: REQ_TEMP_4
   :status: approved
   :priority: mandatory
   :links: REQ_TEMP_3
   :tags: temperature, sensor, api

   **Description:** The component SHALL provide a thread-safe API for consumers to retrieve the
   latest temperature measurement without blocking.

   **Rationale:** Multiple consumers (distance sensor integration, web server, display logic)
   need concurrent access to temperature data from different tasks.

   **Acceptance Criteria:**

   - AC-1: ``temp_sensor_get_latest(temp_measurement_t *out)`` SHALL return most recent reading
   - AC-2: ``temp_measurement_t`` SHALL contain ``temperature_c_x10`` (int16_t, unit 0.1°C, e.g. 215 = 21.5°C) and ``valid`` (bool)
   - AC-3: ``temp_sensor_get_latest()`` SHALL return ``ESP_ERR_INVALID_STATE`` if no measurement has been taken yet
   - AC-4: Function SHALL be thread-safe (mutex-protected)
   - AC-5: ``temp_sensor_start()`` and ``temp_sensor_stop()`` SHALL control the measurement task lifecycle

   **Verification:** Multi-task integration test; mutex verification under concurrent access.


.. req:: Live Temperature Feed to Distance Sensor
   :id: REQ_TEMP_5
   :status: approved
   :priority: mandatory
   :links: REQ_TEMP_4, REQ_SNS_15
   :tags: temperature, integration, distance-sensor

   **Description:** The application SHALL update the distance sensor's temperature compensation
   value dynamically from the temperature sensor component after each successful temperature
   measurement.

   **Rationale:** Static temperature compensation (config default 20°C) introduces distance errors
   of approximately 0.5 mm/m per °C deviation. Live temperature feed eliminates this systematic
   error across seasonal temperature swings.

   **Acceptance Criteria:**

   - AC-1: After each successful temperature measurement, ``distance_sensor_set_temperature()`` SHALL be called
   - AC-2: On startup, distance sensor SHALL use default temperature (200 = 20.0°C) until first DS18B20 reading
   - AC-3: If DS18B20 measurement fails, last known valid temperature SHALL be retained for compensation
   - AC-4: Update SHALL happen in the temperature sensor task after each successful read

   **Verification:** Integration test verifying distance sensor temperature is updated after each
   successful DS18B20 reading; fallback value verified on sensor error.


Traceability
------------

All traceability is automatically generated by Sphinx-Needs based on the ``:links:`` attributes in each requirement.

.. needtable::
   :filter: "REQ_TEMP" in id
   :columns: id, title, status, links

.. needflow::
   :filter: "REQ_TEMP" in id
   :types: req
