Temperature Sensor Design Specification
=========================================

This document specifies the design of the DS18B20 Dallas 1-Wire temperature sensor component.

**Document Version**: 1.0
**Last Updated**: 2026-03-17

Overview
--------

The temperature sensor component uses a single-task architecture with a mutex-protected
shared measurement buffer. It feeds temperature data to the distance sensor for compensation
and provides a non-blocking API for web and display consumers.

Architecture
------------

.. spec:: DS18B20 Component Architecture
   :id: SPEC_TEMP_ARCH_1
   :links: REQ_TEMP_1, REQ_TEMP_4
   :status: approved
   :tags: temperature, architecture, freertos

   **Design:** Single-task component with mutex-protected shared measurement,
   feeding temperature data to distance sensor, web status handler, and display logic.

   **Data Flow:**

   .. code-block:: text

      DS18B20 ──1-Wire──> temp_sensor_task ──mutex──> latest_measurement_t
                               │                              │
                               ▼                              ▼
               distance_sensor_set_temperature()   temp_sensor_get_latest()
                               │                              │
                               ▼                              ▼
                   HC-SR04 distance calculation   web /api/status + display_logic

   **Component Files:**

   - ``temp_sensor.h`` — Public API, data structures, configuration
   - ``temp_sensor.c`` — Measurement task, state management, API implementation
   - ``temp_sensor_sim.c`` — QEMU simulation (compiled when ``CONFIG_TEMP_SENSOR_SIMULATION=y``)

   **FreeRTOS Resources:**

   - Task: ``temp_sensor_task``, core 0, priority 3, stack 2048 bytes
   - Mutex: ``SemaphoreHandle_t`` protecting ``latest_measurement`` struct

   **QEMU Simulation:**

   When ``CONFIG_TEMP_SENSOR_SIMULATION=y`` (set automatically when ``CONFIG_DISTANCE_SENSOR_SIMULATION`` is set):

   - Measurement task sweeps temperature from 0°C → 25°C over 30 seconds, then resets to 0°C
   - Sweep step: ~1.667°C per 2-second measurement cycle (250 steps total in 0.1°C units over 15 cycles)
   - Allows visual verification of the full blue→green→orange LED colour gradient in QEMU
   - No 1-Wire GPIO activity in simulation mode

   **Validation:** Component initialises successfully, task starts, measurements retrieved
   correctly by all consumers in parallel.


Protocol Implementation
-----------------------

.. spec:: Dallas 1-Wire Bit-Banging Implementation
   :id: SPEC_TEMP_ONEWIRE_1
   :links: REQ_TEMP_2
   :status: approved
   :tags: temperature, onewire, protocol, gpio

   **Design:** All 1-Wire timing implemented via ``esp_rom_delay_us()``.
   Interrupt-sensitive timing sections wrapped in ``portDISABLE_INTERRUPTS()`` /
   ``portENABLE_INTERRUPTS()`` to prevent measurement jitter.

   **GPIO Configuration (open-drain emulation):**

   .. code-block:: c

      /* Pull LOW: set as output and drive low */
      gpio_set_direction(pin, GPIO_MODE_OUTPUT);
      gpio_set_level(pin, 0);

      /* Release (high-impedance): set as input, pull-up keeps line HIGH */
      gpio_set_direction(pin, GPIO_MODE_INPUT);
      /* Internal pull-up configured at init as GPIO_PULLUP_ONLY */

   **Timing Table (all durations in µs):**

   ========================  ===========  ===========================
   Operation                 Pull LOW     Release / Sample
   ========================  ===========  ===========================
   Reset pulse               ≥480 µs      Release, wait 60 µs, sample
   Write 1 slot              6 µs         Release, wait 64 µs
   Write 0 slot              60 µs        Release, wait 10 µs
   Read slot                 6 µs         Release, wait 9 µs, sample, wait 55 µs
   ========================  ===========  ===========================

   **Reset / Presence Detection:**

   .. code-block:: c

      static bool onewire_reset(gpio_num_t pin) {
          portDISABLE_INTERRUPTS();
          gpio_set_direction(pin, GPIO_MODE_OUTPUT);
          gpio_set_level(pin, 0);
          esp_rom_delay_us(480);
          gpio_set_direction(pin, GPIO_MODE_INPUT);
          esp_rom_delay_us(70);
          bool presence = (gpio_get_level(pin) == 0);  /* DS18B20 pulls LOW */
          esp_rom_delay_us(410);
          portENABLE_INTERRUPTS();
          return presence;
      }

   **DS18B20 Command Sequence:**

   1. Reset → presence check
   2. Write ``0xCC`` (Skip ROM) — single sensor, no ROM matching
   3. Write ``0x44`` (Convert T) — initiate conversion
   4. ``vTaskDelay(800ms / portTICK_PERIOD_MS)`` — await ≥750ms conversion
   5. Reset → presence check
   6. Write ``0xCC`` (Skip ROM)
   7. Write ``0xBE`` (Read Scratchpad)
   8. Read 9 bytes
   9. Verify CRC8 (byte 8 = CRC of bytes 0–7)

   **Temperature Decoding:**

   .. code-block:: c

      /* Bytes 0 + 1 from scratchpad: 16-bit little-endian twos-complement */
      int16_t raw = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);
      /* 12-bit resolution: raw LSB = 0.0625°C */
      /* Convert to 0.1°C integer units: multiply by 10, divide by 16 */
      int16_t temp_c_x10 = (raw * 10) / 16;

   **CRC8 (Dallas/Maxim):**

   - Polynomial: x⁸ + x⁵ + x⁴ + 1 (0x31)
   - Computed over bytes 0–7 of scratchpad; must match byte 8
   - Invalid CRC → measurement discarded, previous value retained

   **Validation:** Logic analyser confirms timing compliance; CRC validation catches wiring errors.


Task and Integration Design
---------------------------

.. spec:: Temperature Measurement Task Design
   :id: SPEC_TEMP_TASK_1
   :links: REQ_TEMP_3, REQ_TEMP_5
   :status: approved
   :tags: temperature, freertos, task, integration

   **Design:** FreeRTOS task with explicit convert-wait-read sequence, followed by
   updating the distance sensor compensation value.

   **Task Loop (pseudo-code):**

   .. code-block:: text

      while (running) {
          tick_start = xTaskGetTickCount()

          if (!onewire_reset(data_pin)) {
              ESP_LOGW(TAG, "DS18B20 not detected");
              goto sleep;
          }
          onewire_write(0xCC);      /* Skip ROM */
          onewire_write(0x44);      /* Convert T */

          vTaskDelay(800ms)         /* ← ≥750ms at 12-bit resolution */

          onewire_reset(data_pin);
          onewire_write(0xCC);      /* Skip ROM */
          onewire_write(0xBE);      /* Read Scratchpad */
          onewire_read_bytes(scratchpad, 9);

          if (crc8_valid(scratchpad)) {
              temp = decode_temperature(scratchpad);
              mutex_lock();
              latest.temperature_c_x10 = temp;
              latest.valid = true;
              latest.timestamp_us = esp_timer_get_time();
              mutex_unlock();
              distance_sensor_set_temperature(temp);   /* REQ_TEMP_5 */
          } else {
              ESP_LOGW(TAG, "CRC invalid, retaining last value");
          }

      sleep:
          remaining = interval_ticks - (xTaskGetTickCount() - tick_start);
          if (remaining > 0) vTaskDelay(remaining);
      }

   **Error Handling:**

   - No presence pulse: log warning, skip measurement update, retain last valid value
   - CRC mismatch: log warning, skip update, retain last valid value
   - 5 consecutive failures: log error (continue retrying, do not stop task)

   **Distance Sensor Integration:**

   ``distance_sensor_set_temperature()`` is called directly from the measurement task
   after each successful read. No separate integration callback or timer is needed.

   **Validation:** Integration test verifying temperature updates propagate to
   distance sensor; error injection test confirming last-valid retention behavior.


API Design
----------

.. spec:: Temperature Sensor Public API
   :id: SPEC_TEMP_API_1
   :links: REQ_TEMP_1, REQ_TEMP_4
   :status: approved
   :tags: temperature, api, data-structures

   **Data Structures:**

   .. code-block:: c

      /** @brief Temperature sensor configuration */
      typedef struct {
          gpio_num_t data_pin;    ///< 1-Wire data GPIO (default: GPIO4, from config key "temp_gpio")
          uint32_t   interval_ms; ///< Measurement interval ms (default: 2000, from config key "temp_interval_ms")
      } temp_sensor_config_t;

      /** @brief Single temperature measurement */
      typedef struct {
          int16_t  temperature_c_x10; ///< Temperature in 0.1°C units (215 = 21.5°C)
          bool     valid;             ///< true if successfully measured, false on startup or error
          uint64_t timestamp_us;      ///< Measurement time from esp_timer_get_time()
      } temp_measurement_t;

   **Public API:**

   .. code-block:: c

      /** Initialise component. Pass NULL to use config_manager values. */
      esp_err_t temp_sensor_init(const temp_sensor_config_t *config);

      /** Create and start measurement task. */
      esp_err_t temp_sensor_start(void);

      /** Stop and delete measurement task. */
      esp_err_t temp_sensor_stop(void);

      /**
       * Get the latest measurement (non-blocking, thread-safe).
       * Returns ESP_ERR_INVALID_STATE if no measurement available yet.
       */
      esp_err_t temp_sensor_get_latest(temp_measurement_t *out);

   **Configuration Loading (in temp_sensor_init with NULL config):**

   .. code-block:: c

      int32_t gpio_num = 4;
      int32_t interval_ms = 2000;
      config_get_int32("temp_gpio", &gpio_num);
      config_get_int32("temp_interval_ms", &interval_ms);

   **Validation:** API functions return correct error codes; thread-safety verified by
   concurrent consumer test.
