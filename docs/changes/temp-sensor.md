# Change Document: temp-sensor

**Status**: approved — RST files updated, ready for implementation
**Branch**: feature/temp-sensor
**Created**: 2026-03-17
**Author**: GitHub Copilot

---

## Summary

Hinzufügen eines DS18B20 Dallas 1-Wire Temperatursensors. Der Sensor dient drei Zwecken:
1. **Temperaturkompensation** des HC-SR04 Ultraschallsensors (Schallgeschwindigkeit)
2. **Temperaturanzeige** im Web-Interface (Status-Seite und Settings)
3. **Temperaturbasierte LED-Farbe** der Positions-LED: Blau bei Kälte (< 5°C), Orange bei Wärme (> 20°C); Schwellenwerte konfigurierbar

---

## Level 0: User Stories

**Status**: ✅ completed

### Impacted User Stories

| ID | Title | Impact | Notes |
|----|-------|--------|-------|
| US_DISPLAY_1 | Visual Distance Feedback for Parking | modified | LED-Farbe der Positionsanzeige ändert sich zusätzlich basierend auf Temperatur |
| US_SETUP_1 | Device Setup via Web Interface | modified | Web-UI zeigt Temperatur an; neue Konfigurationsparameter für Temperaturschwellen |
| US_DEV_1 | Modular Firmware Extensibility | modified | Neues `temp_sensor`-Komponent dient als weiteres Referenzbeispiel |

### New User Stories

```rst
.. story:: Temperature-Aware Parking Environment Indicator
   :id: US_TEMP_1
   :status: draft
   :links: US_DISPLAY_1, US_SETUP_1
   :tags: temperature, sensor, garage-user, display

   **Role:** Garage user

   **Story:**
   As a garage user, I want the device to measure the garage temperature and
   show it on the web interface and through the LED color, so that I can see
   at a glance whether it is cold (blue) or warm (orange) in the garage.

   **Value:**
   In a garage parking aid context, temperature affects both sensor accuracy and
   usability — icy conditions (<5°C) are relevant for safety. The LED position
   indicator double-codes distance (position) and temperature (color), providing
   ambient context without an extra display. The web interface gives precise
   temperature readings for monitoring.
```

### Decisions

- **D0-1**: Neues `US_TEMP_1` wird benötigt, da die temperaturbasierte LED-Farbe und das Temperature-Monitoring deutlich über den bisherigen Scope von US_DISPLAY_1 (reine Distanzanzeige) hinausgehen.
- **D0-2**: US_DISPLAY_1 wird als "modified" markiert, weil links zu neuen Temperature-Requirements hinzugefügt werden müssen.
- **D0-3**: US_SETUP_1 wird als "modified" markiert, weil der Web-UI-Scope neue Konfigurationsfelder (Temperaturschwellen) umfasst.
- **D0-4**: US_DEV_1 wird als "modified" markiert, weil ein neues Referenzkomponent (`temp_sensor`) hinzukommt, auf das die Story verweisen soll.

### Horizontal Check (MECE)

- [x] Keine Widersprüche mit bestehenden User Stories
- [x] Keine Redundanzen: US_TEMP_1 beschreibt spezifisch den Temperaturkontext; US_DISPLAY_1 bleibt auf Distanzanzeige fokussiert
- [x] Lücken abgedeckt: Kompensation (technisch intern), Anzeige (Web UI), Farbe (LED) — alle drei Aspekte der Anforderung sind durch US_TEMP_1 + modifizierte US abgedeckt

---

## Level 1: Requirements

**Status**: ✅ completed

### Impacted Requirements (via US-Links)

| ID | Linked From | Impact | Notes |
|----|-------------|--------|-------|
| `REQ_DSP_3` | US_DISPLAY_1 | modified | Sagt "single LED in **green**" — muss "temperature-based color" werden |
| `REQ_DSP_2` | US_DISPLAY_1 | modified | Neue Konfig-Parameter `temp_cold_c10` und `temp_warm_c10` hinzufügen |
| `REQ_WEB_1` | US_SETUP_1 | modified | Temperaturanzeige wird durch neues REQ_WEB_TEMP_1 konkretisiert |
| `REQ_SNS_11` | US_DISPLAY_1 | modified | temperature_c_x10 kommt jetzt dynamisch vom DS18B20, nicht statisch |

### New Requirements

#### REQ_SYS_TEMP_1 (new in req_system.rst)

```rst
.. req:: Temperature Sensing Capability
   :id: REQ_SYS_TEMP_1
   :status: draft
   :priority: mandatory
   :links: US_TEMP_1
   :tags: system, temperature, sensor

   **Description:**
   The system SHALL incorporate a DS18B20 digital temperature sensor to measure
   ambient garage temperature. The measurement SHALL serve two purposes: improving
   distance sensor accuracy through temperature compensation, and providing
   temperature information to the web interface and LED display.

   **Rationale:**
   Garage environments vary from sub-zero to summer heat. Temperature-corrected
   distance measurements give parking accuracy across seasons. Ambient display
   colour provides safety-relevant cold-weather indication.

   **Acceptance Criteria:**

   * AC-1: System provides temperature readings in the DS18B20 operating range (-55°C to +125°C)
   * AC-2: Temperature is applied to distance sensor speed-of-sound compensation in real time
   * AC-3: Temperature is displayed on the web status page and accessible via REST API
   * AC-4: Temperature affects the position LED colour
```

#### REQ_TEMP_1 (new file req_temp_sensor.rst)

```rst
.. req:: DS18B20 Component Initialization
   :id: REQ_TEMP_1
   :status: draft
   :priority: mandatory
   :links: REQ_SYS_TEMP_1
   :tags: temperature, sensor, initialization, gpio

   **Description:**
   The temperature sensor component SHALL provide initialization via
   ``temp_sensor_init(const temp_sensor_config_t *config)`` that configures the
   GPIO data pin, initialises internal state, and uses default configuration when
   no config is provided.

   **Rationale:**
   Deterministic startup sequence required for reliable sensor operation and
   system integration.

   **Acceptance Criteria:**

   - AC-1: ``temp_sensor_init(NULL)`` SHALL return ``ESP_OK`` and use GPIO4 as default data pin
   - AC-2: Data pin SHALL be configured as open-drain output with internal pull-up (4.7kΩ external pull-up recommended but not enforced in firmware)
   - AC-3: Function SHALL return appropriate ``esp_err_t`` codes for error conditions
   - AC-4: Component SHALL support a single DS18B20 sensor
```

#### REQ_TEMP_2 (new file req_temp_sensor.rst)

```rst
.. req:: Dallas 1-Wire Protocol Communication
   :id: REQ_TEMP_2
   :status: draft
   :priority: mandatory
   :links: REQ_TEMP_1
   :tags: temperature, sensor, onewire, protocol

   **Description:**
   The component SHALL implement Dallas 1-Wire bit-banging protocol over a single
   GPIO data pin to communicate with the DS18B20 sensor.

   **Rationale:**
   DS18B20 exclusively uses 1-Wire protocol. Bit-banging avoids hardware peripheral
   dependencies and works on any GPIO.

   **Acceptance Criteria:**

   - AC-1: Reset pulse SHALL conform to DS18B20 datasheet (480µs LOW, release, 60µs sample window)
   - AC-2: Write-1 and Write-0 slots SHALL meet DS18B20 timing requirements
   - AC-3: Read slots SHALL capture valid data bits within DS18B20 timing window
   - AC-4: 12-bit resolution SHALL be used for ±0.0625°C measurement precision
   - AC-5: Timing-critical sections SHALL disable interrupts to prevent jitter
```

#### REQ_TEMP_3 (new file req_temp_sensor.rst)

```rst
.. req:: Periodic Temperature Measurement Task
   :id: REQ_TEMP_3
   :status: draft
   :priority: mandatory
   :links: REQ_TEMP_2
   :tags: temperature, sensor, measurement, freertos

   **Description:**
   The component SHALL run a FreeRTOS task that periodically initiates DS18B20
   temperature conversion and retrieves the result.

   **Rationale:**
   DS18B20 conversion takes ≥750ms at 12-bit resolution. A dedicated task allows
   correct sequencing (initiate → wait → read) without blocking other components.

   **Acceptance Criteria:**

   - AC-1: Default measurement interval SHALL be 2000ms (0.5 Hz)
   - AC-2: Interval SHALL be configurable via ``temp_interval_ms`` config key
   - AC-3: Task SHALL wait at least 750ms after conversion start before reading result
   - AC-4: Task SHALL use ``vTaskDelay`` during conversion wait (no busy-wait)
   - AC-5: Task stack size SHALL be ≤ 2048 bytes
```

#### REQ_TEMP_4 (new file req_temp_sensor.rst)

```rst
.. req:: Temperature Data API
   :id: REQ_TEMP_4
   :status: draft
   :priority: mandatory
   :links: REQ_TEMP_3
   :tags: temperature, sensor, api

   **Description:**
   The component SHALL provide a thread-safe API for consumers to retrieve the
   latest temperature measurement without blocking.

   **Rationale:**
   Multiple consumers (distance sensor integration, web server, display logic)
   need concurrent access to temperature data.

   **Acceptance Criteria:**

   - AC-1: ``temp_sensor_get_latest(temp_measurement_t *out)`` SHALL return most recent reading
   - AC-2: ``temp_measurement_t`` SHALL contain ``temperature_c_x10`` (int16_t, unit 0.1°C, e.g. 215 = 21.5°C) and ``valid`` (bool)
   - AC-3: ``temp_sensor_get_latest()`` SHALL return ``ESP_ERR_INVALID_STATE`` if no measurement has been taken yet
   - AC-4: Function SHALL be thread-safe (mutex-protected or atomic)
   - AC-5: ``temp_sensor_start()`` and ``temp_sensor_stop()`` SHALL control the measurement task lifecycle
```

#### REQ_TEMP_5 (new file req_temp_sensor.rst)

```rst
.. req:: Live Temperature Feed to Distance Sensor
   :id: REQ_TEMP_5
   :status: draft
   :priority: mandatory
   :links: REQ_TEMP_4, REQ_SNS_15
   :tags: temperature, integration, distance-sensor

   **Description:**
   The application SHALL update the distance sensor's temperature compensation
   value dynamically from the temperature sensor component after each successful
   temperature measurement.

   **Rationale:**
   Static temperature compensation (config default 20°C) introduces distance
   errors of ≈0.5mm/cm per °C deviation. Live temperature feed eliminates this
   systematic error across seasonal temperature swings.

   **Acceptance Criteria:**

   - AC-1: After each successful temperature measurement, ``distance_sensor_set_temperature()`` SHALL be called
   - AC-2: On startup, distance sensor SHALL use default temperature (200 = 20.0°C) until first DS18B20 reading
   - AC-3: If DS18B20 measurement fails, last known valid temperature SHALL be retained for compensation
   - AC-4: Update SHALL happen in the temperature sensor task or a dedicated integration callback
```

#### REQ_SNS_15 (new addition to req_distance_sensor.rst)

```rst
.. req:: Dynamic Temperature Compensation Update
   :id: REQ_SNS_15
   :status: draft
   :priority: mandatory
   :links: REQ_SNS_11
   :tags: sensor, temperature, api

   **Description:**
   The distance sensor component SHALL provide ``distance_sensor_set_temperature(int16_t temperature_c_x10)``
   to allow runtime updates of the temperature compensation value without restarting the sensor.

   **Rationale:**
   Enables the temperature sensor component to feed live readings into the
   distance calculation without requiring sensor restart or NVS access.

   **Acceptance Criteria:**

   - AC-1: Function SHALL update the internal compensation value used in the next measurement calculation
   - AC-2: Function SHALL be thread-safe (callable from any task)
   - AC-3: Updated value SHALL take effect on the next triggered measurement
   - AC-4: Function SHALL return ``ESP_OK`` on success, ``ESP_ERR_INVALID_STATE`` if sensor not initialised
```

#### REQ_DSP_6 (new addition to req_display.rst)

```rst
.. req:: Temperature-Based Position LED Colour
   :id: REQ_DSP_6
   :status: draft
   :priority: mandatory
   :links: REQ_DSP_3, REQ_TEMP_4, REQ_CFG_JSON_16
   :tags: display, temperature, colour, ux

   **Description:**
   The display system SHALL apply a temperature-dependent colour to the position
   indicator LED using a blue→green→orange gradient, with configurable cold and
   warm temperature thresholds.

   **Rationale:**
   The LED position already encodes distance; using colour to encode temperature
   provides a second channel of information without adding hardware. Blue for cold
   (< 5°C) is a universally understood safety signal for icy garage conditions.
   Orange for warm (> 20°C) is an ambient indicator. The gradient between thresholds
   gives a continuous, intuitive readout.

   **Acceptance Criteria:**

   - AC-1: At or below ``temp_cold_c10`` threshold the position LED SHALL be blue (0, 0, 255)
   - AC-2: At or above ``temp_warm_c10`` threshold the position LED SHALL be orange (255, 165, 0)
   - AC-3: Between thresholds the colour SHALL be a linear interpolation between blue and orange via green
   - AC-4: Both thresholds SHALL be read from the configuration system (keys ``temp_cold_c10`` and ``temp_warm_c10``)
   - AC-5: If no valid temperature reading is available, the position LED SHALL default to green (original behaviour maintained)
   - AC-6: Colour mapping SHALL NOT affect the warning colour logic of REQ_DSP_4 (too close → red) or REQ_DSP_5 (out of range → red)
```

#### REQ_WEB_TEMP_1 (new addition to req_web_server.rst)

```rst
.. req:: Temperature Status Display
   :id: REQ_WEB_TEMP_1
   :status: draft
   :priority: mandatory
   :links: REQ_WEB_1, REQ_TEMP_4
   :tags: web, temperature, ui, monitoring

   **Description:**
   The web status page SHALL display the current temperature reading, and the
   ``/api/status`` REST endpoint SHALL include temperature in its JSON payload.

   **Rationale:**
   Temperature monitoring via web interface allows garage users to check
   conditions remotely without physical presence.

   **Acceptance Criteria:**

   - AC-1: Status page SHALL display temperature as "XX.X °C" (one decimal place)
   - AC-2: ``GET /api/status`` SHALL return JSON including ``"temperature_c": <float>``
   - AC-3: If no valid reading available, page SHALL show "-- °C" and API SHALL return ``"temperature_c": null``
   - AC-4: Temperature display SHALL update on page reload (no SSE/WebSocket required)
```

#### REQ_CFG_JSON_16 (new addition to req_config_manager_json.rst)

```rst
.. req:: Temperature Sensor Configuration Parameters
   :id: REQ_CFG_JSON_16
   :status: draft
   :priority: mandatory
   :links: REQ_CFG_JSON_1, REQ_DSP_6, REQ_TEMP_1, REQ_TEMP_3
   :tags: config, temperature, thresholds, gpio

   **Description:**
   The configuration schema SHALL include parameters for the temperature sensor
   GPIO pin, measurement interval, and colour thresholds used by the display system.

   **Rationale:**
   Different deployments need different GPIO assignments and climate-appropriate
   colour thresholds (Scandinavian garage vs Mediterranean garage).

   **Acceptance Criteria:**

   - AC-1: ``temp_gpio`` SHALL be an integer config parameter, default 4, range 0–39; applied on boot
   - AC-2: ``temp_interval_ms`` SHALL be an integer config parameter (ms), default 2000, range 500–60000
   - AC-3: ``temp_cold_c10`` SHALL be an integer config parameter (unit: 0.1°C), default 50 (= 5.0°C), range -100 to 500
   - AC-4: ``temp_warm_c10`` SHALL be an integer config parameter (unit: 0.1°C), default 200 (= 20.0°C), range -100 to 500
   - AC-5: All four parameters SHALL appear in a "Temperature Sensor" group in the web settings page
   - AC-6: Validation SHALL ensure ``temp_cold_c10`` < ``temp_warm_c10``
   - AC-7: Parameters SHALL be stored in NVS under their respective keys
```

### Modified Requirement Details

#### REQ_DSP_3 — modified: change "green" to "temperature-based colour"

Current AC-1/description says "illuminate a single LED **in green**". Change to "**in a colour determined by the current temperature** per REQ_DSP_6."

#### REQ_DSP_2 — modified: add temp threshold config parameters

Add to Configuration Parameters list:
- ``temp_cold_c10``: Cold temperature threshold (tenths of °C, default 50)
- ``temp_warm_c10``: Warm temperature threshold (tenths of °C, default 200)

#### REQ_SNS_11 — modified: note that temperature_c_x10 is dynamically updated

Add AC: "The distance sensor's ``temperature_c_x10`` compensation value SHALL be updatable at runtime via ``distance_sensor_set_temperature()`` (REQ_SNS_15)"

### Decisions

- **D1-1**: GPIO4 assigned as DS18B20 data pin (default, configurable)
- **D1-2**: Blue→green→orange gradient between thresholds (linear interpolation)
- **D1-3**: Both status page HTML and /api/status REST endpoint include temperature
- **D1-4**: No new `temp_interval_ms` config visible in web UI — only code-configurable (simplify UX)
  - **Revision**: Actually temperature measurement interval should be configurable via config schema too → REQ_TEMP_3 AC-2 references ``temp_interval_ms`` config key, and REQ_CFG_JSON_16 should include it
- **D1-5**: On startup, fallback temperature = 20°C until first DS18B20 reading
- **D1-6**: Simulator: DS18B20 not modeled in QEMU — component in simulation returns a static default (configurable via build-time define), similar to distance_sensor_sim.c pattern

### Horizontal Check (MECE)

- [x] REQ_SNS_15 (new distance sensor API) does not overlap with any existing REQ_SNS_* 
- [x] REQ_DSP_6 scoped to position LED only — does not conflict with REQ_DSP_4/REQ_DSP_5 (red warning)
- [x] REQ_TEMP_5 integration requirement gaps bridged: it references both REQ_TEMP_4 (API) and REQ_SNS_15 (target)
- [x] REQ_CFG_JSON_16 covers all new config keys: `temp_cold_c10`, `temp_warm_c10`, `temp_interval_ms`
- [x] REQ_WEB_TEMP_1 is distinct from REQ_WEB_1 (general status) — no redundancy
- [ ] **Gap identified**: REQ_CFG_JSON_16 should also include `temp_gpio` (GPIO pin configurable) and `temp_interval_ms` — updating in place

---

## Level 2: Design

**Status**: ✅ completed

### Impacted Design Specs (via REQ links)

| ID | Title | Impact | Notes |
|----|-------|--------|-------|
| `SPEC_DSP_ALGO_1` | Distance-to-Visual Mapping Algorithm | modified | "single LED green" → colour from SPEC_DSP_TEMP_1 |
| `SPEC_DSP_ARCH_2` | Configuration Integration | modified | Add `temp_cold_c10` / `temp_warm_c10` to cached config values |
| `SPEC_WEB_ROUTES_1` | URI Routing Table | modified | `/api/status` description: add temperature field |

### New Design Specs

#### SPEC_TEMP_ARCH_1 (new file spec_temp_sensor.rst)

```rst
.. spec:: DS18B20 Component Architecture
   :id: SPEC_TEMP_ARCH_1
   :links: REQ_TEMP_1, REQ_TEMP_4
   :status: draft
   :tags: temperature, architecture, freertos

   **Design:** Single-task component with mutex-protected shared measurement,
   feeding temperature data to distance sensor and status API.

   **Data Flow:**

   .. code-block:: text

      DS18B20 ──1-Wire──> temp_sensor_task ──mutex──> latest_measurement_t
                               │                              │
                               ▼                              ▼
               distance_sensor_set_temperature()   temp_sensor_get_latest()
                               │                              │
                               ▼                              ▼
                   distance calculation            web /api/status + display

   **Component Structure:**

   - ``temp_sensor.h`` / ``temp_sensor.c``: Public API, task, state machine
   - ``onewire.h`` / ``onewire.c``: 1-Wire bit-banging protocol layer
   - Internal mutex: ``SemaphoreHandle_t`` protecting ``latest_measurement``
   - Task: ``temp_sensor_task`` on core 0, priority 3, stack 2048 bytes

   **QEMU Simulation:**

   - Component compiles with ``CONFIG_TEMP_SENSOR_SIMULATION=y`` Kconfig flag
   - Simulator sweeps temperature 0°C → 25°C over 30 seconds, then resets to 0°C
   - Sweep rate: +0.833°C per second (≈ 8.33 in ``temperature_c_x10`` per second)
   - At each measurement cycle (default 2s): advance by ~1.667°C (16.67 in c_x10)
   - Allows visual verification of the full blue→green→orange colour gradient in QEMU
   - Follows same pattern as ``distance_sensor_sim.c``

   **Validation:** Component initialises, task starts, measurements retrieved
   correctly by all consumers.
```

#### SPEC_TEMP_ONEWIRE_1 (new file spec_temp_sensor.rst)

```rst
.. spec:: Dallas 1-Wire Bit-Banging Implementation
   :id: SPEC_TEMP_ONEWIRE_1
   :links: REQ_TEMP_2
   :status: draft
   :tags: temperature, onewire, protocol, gpio

   **Design:** All 1-Wire timing implemented via ``esp_rom_delay_us()``.
   Interrupt-sensitive sections wrapped in ``portDISABLE_INTERRUPTS()`` /
   ``portENABLE_INTERRUPTS()`` to prevent jitter.

   **GPIO Configuration:**

   .. code-block:: c

      // Data line as open-drain: set direction to output-low to pull low,
      // set direction to input (high-impedance) to release (external 4.7kΩ pull-up)
      gpio_set_pull_mode(data_pin, GPIO_PULLUP_ONLY);  // internal weak pull-up backup
      gpio_set_direction(data_pin, GPIO_MODE_INPUT);    // default: released

   **Key Timings (all in µs):**

   ========================  =========  ==========
   Operation                 Pull LOW   Release/Wait
   ========================  =========  ==========
   Reset pulse               ≥480µs     Release, wait 60µs, sample
   Write 1 slot              6µs        Release, wait 64µs
   Write 0 slot              60µs       Release, wait 10µs
   Read slot                 6µs        Release, wait 9µs, sample, wait 55µs
   ========================  =========  ==========

   **Reset/Presence Detection:**

   .. code-block:: c

      bool onewire_reset(gpio_num_t pin) {
          portDISABLE_INTERRUPTS();
          gpio_set_direction(pin, GPIO_MODE_OUTPUT);
          gpio_set_level(pin, 0);
          esp_rom_delay_us(480);
          gpio_set_direction(pin, GPIO_MODE_INPUT);
          esp_rom_delay_us(70);
          bool presence = (gpio_get_level(pin) == 0);  // DS18B20 pulls low
          esp_rom_delay_us(410);
          portENABLE_INTERRUPTS();
          return presence;
      }

   **DS18B20 Commands:**

   - Skip ROM (0xCC): Single device on bus, no ROM matching needed
   - Convert T (0x44): Initiate temperature conversion
   - Read Scratchpad (0xBE): Read 9 bytes from internal register

   **Temperature Decoding:**

   .. code-block:: c

      // Bytes 0+1 from scratchpad: raw 16-bit twos-complement value
      // For 12-bit resolution: LSB = bit0..3 fractional, bits 4..14 integer
      int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
      // Convert to 0.1°C units: multiply by 10, then divide by 16
      int16_t temp_c_x10 = (raw * 10) / 16;

   **Validation:** Logic analyser traces confirm timing compliance;
   presence pulse detected on valid hardware.
```

#### SPEC_TEMP_TASK_1 (new file spec_temp_sensor.rst)

```rst
.. spec:: Temperature Measurement Task Design
   :id: SPEC_TEMP_TASK_1
   :links: REQ_TEMP_3, REQ_TEMP_5
   :status: draft
   :tags: temperature, freertos, task, integration

   **Design:** FreeRTOS task with explicit conversion-wait-read sequence;
   updates distance sensor temperature after each valid reading.

   **Task Loop:**

   .. code-block:: text

      loop forever:
        1. onewire_reset() → presence check
        2. Write Skip ROM (0xCC) + Convert T (0x44)
        3. vTaskDelay(800ms)    ← DS18B20 12-bit conversion ≥750ms
        4. onewire_reset()
        5. Write Skip ROM + Read Scratchpad (0xBE)
        6. Read 9 bytes, verify CRC8
        7. Decode temperature → temperature_c_x10
        8. Lock mutex → update latest_measurement (valid=true)
        9. distance_sensor_set_temperature(temperature_c_x10)  ← REQ_TEMP_5
       10. vTaskDelay(remaining_interval - 800ms)

   **Error Handling:**

   - No presence pulse → log warning, skip update, retain last valid value
   - CRC mismatch → log warning, skip update, retain last valid value
   - On repeated failure (5 consecutive): log error, continue retrying

   **CRC8 Validation:**

   - Dallas/Maxim CRC8 polynomial: 0x31 (x^8 + x^5 + x^4 + 1)
   - Computed over bytes 0–7 of scratchpad, must match byte 8

   **Validation:** Task correctly sequences conversion and read; distance sensor
   temperature updated after each measurement; no deadlock possible (mutex
   held only during short copy operation).
```

#### SPEC_TEMP_API_1 (new file spec_temp_sensor.rst)

```rst
.. spec:: Temperature Sensor Public API
   :id: SPEC_TEMP_API_1
   :links: REQ_TEMP_1, REQ_TEMP_4
   :status: draft
   :tags: temperature, api, data-structures

   **Data Structures:**

   .. code-block:: c

      /** Temperature sensor configuration */
      typedef struct {
          gpio_num_t data_pin;         ///< 1-Wire data GPIO pin (default GPIO4)
          uint32_t   interval_ms;      ///< Measurement interval in ms (default 2000)
      } temp_sensor_config_t;

      /** Single temperature measurement */
      typedef struct {
          int16_t  temperature_c_x10;  ///< Temperature in 0.1°C units (e.g. 215 = 21.5°C)
          bool     valid;              ///< true if value was successfully measured
          uint64_t timestamp_us;       ///< esp_timer_get_time() at measurement
      } temp_measurement_t;

   **Public Functions:**

   .. code-block:: c

      esp_err_t temp_sensor_init(const temp_sensor_config_t *config);
      esp_err_t temp_sensor_start(void);
      esp_err_t temp_sensor_stop(void);
      esp_err_t temp_sensor_get_latest(temp_measurement_t *out);

   **Configuration Source:**

   Config reads from config_manager automatically in ``temp_sensor_init(NULL)``:

   .. code-block:: c

      config_get_int32("temp_gpio", &gpio);            // default 4
      config_get_int32("temp_interval_ms", &interval); // default 2000

   **Validation:** API functions return correct error codes;
   thread-safety verified by concurrent caller test.
```

#### SPEC_DSP_TEMP_1 (new addition to spec_display.rst)

```rst
.. spec:: Temperature-Based Colour Mapping Algorithm
   :id: SPEC_DSP_TEMP_1
   :links: REQ_DSP_6
   :status: draft
   :tags: display, temperature, colour, algorithm

   **Design:** Linear RGB gradient in two segments:
   Cold (blue) → Mid (green) → Warm (orange), using integer arithmetic.

   **Colour Anchors:**

   ========================  ==========  ============
   State                     Condition   RGB value
   ========================  ==========  ============
   Cold                      T ≤ cold    (  0,   0, 255)
   Green midpoint            T = mid     (  0, 255,   0)
   Warm                      T ≥ warm    (255, 165,   0)
   No valid reading          --          (  0, 255,   0)  ← fallback green
   ========================  ==========  ============

   Where ``mid_c10 = (temp_cold_c10 + temp_warm_c10) / 2``.

   **Integer Algorithm:**

   .. code-block:: c

      void temp_to_rgb(int16_t temp, int16_t cold, int16_t warm,
                       uint8_t *r, uint8_t *g, uint8_t *b) {
          if (temp <= cold) { *r=0;   *g=0;   *b=255; return; }
          if (temp >= warm) { *r=255; *g=165; *b=0;   return; }
          int16_t mid = (cold + warm) / 2;
          if (temp <= mid) {
              // Blue → Green: r stays 0, g increases, b decreases
              uint16_t p = (uint16_t)((temp - cold) * 1000 / (mid - cold));
              *r = 0;
              *g = (uint8_t)(p * 255 / 1000);
              *b = (uint8_t)(255 - p * 255 / 1000);
          } else {
              // Green → Orange: r increases, g decreases from 255→165, b stays 0
              uint16_t p = (uint16_t)((temp - mid) * 1000 / (warm - mid));
              *r = (uint8_t)(p * 255 / 1000);
              *g = (uint8_t)(255 - p * 90 / 1000);   // 255→165 = -90
              *b = 0;
          }
      }

   **Caller:**

   ``display_task()`` calls ``temp_to_rgb()`` with ``temp_sensor_get_latest()`` result
   before calling ``led_controller_set_pixel()``. If ``valid == false``, defaults to
   green (R=0, G=255, B=0).

   **No-display conditions (REQ_DSP_4 / REQ_DSP_5 override):**

   The temperature colour function is called **only** for the valid distance range
   (green zone). Red warning states (too close / out of range) are unaffected.

   **Validation:** Unit test with boundary values (cold, mid, warm ±1) verifying
   RGB values match expected anchors; no division-by-zero when cold == warm via
   config validation.
```

#### SPEC_WEB_TEMP_1 (new addition to spec_web_server.rst)

```rst
.. spec:: Temperature Field in /api/status Response
   :id: SPEC_WEB_TEMP_1
   :links: REQ_WEB_TEMP_1
   :status: draft
   :tags: web, temperature, api, json

   **Design:** Extend ``wifi_status_handler()`` to include temperature reading
   from ``temp_sensor_get_latest()``.

   **Extended JSON Response:**

   .. code-block:: json

      {
        "wifi_mode": "STA",
        "ssid": "MyNetwork",
        "rssi": -55,
        "ip_address": "192.168.1.42",
        "temperature_c": 12.5
      }

   **Null case (no valid reading yet):**

   .. code-block:: json

      {
        "wifi_mode": "STA",
        ...
        "temperature_c": null
      }

   **Implementation:**

   .. code-block:: c

      temp_measurement_t temp;
      if (temp_sensor_get_latest(&temp) == ESP_OK && temp.valid) {
          cJSON_AddNumberToObject(json, "temperature_c",
                                  temp.temperature_c_x10 / 10.0);
      } else {
          cJSON_AddNullToObject(json, "temperature_c");
      }

   **HTML Status Page:**

   The dashboard (``index.html``) fetches ``/api/status`` and renders:

   .. code-block:: html

      <div class="status-item">
        <span class="label">Temperatur</span>
        <span class="value" id="temperature">-- °C</span>
      </div>

   JavaScript updates ``#temperature`` from ``data.temperature_c`` (formats to
   one decimal place, shows ``"-- °C"`` if null).

   **Validation:** API returns correct value after first measurement; null returned
   before first reading; HTML field updates on page refresh.
```

### Modified Spec Details

#### SPEC_DSP_ALGO_1 — modified: LED colour is temperature-based

Add to **Behaviors** section:
> "Position LED colour is determined by ``SPEC_DSP_TEMP_1`` temperature mapping, not fixed green."

Change current text "single LED green illuminated at mapped position" to:
> "Single LED illuminated at mapped position with temperature-dependent colour (``SPEC_DSP_TEMP_1``); falls back to green if no valid temperature reading."

#### SPEC_DSP_ARCH_2 — modified: cache temperature thresholds

Add to **Implementation** section:
```c
// New cached config values at task startup:
config_get_int32("temp_cold_c10", &temp_cold_c10);  // default 50
config_get_int32("temp_warm_c10", &temp_warm_c10);  // default 200
```

#### SPEC_WEB_ROUTES_1 — modified: update /api/status description

Change `/api/status` entry from "WiFi status (SSID, RSSI, mode)" to "WiFi + temperature status".

### Decisions

- **D2-1**: Bit-banging over native RMT/1-Wire peripheral — simpler, no peripheral conflicts, GPIO4 is fine for 2s measurement rate
- **D2-2**: CRC8 validation on all DS18B20 reads — catches wiring noise without expensive retry mechanism
- **D2-3**: Temperature stored as `int16_t temperature_c_x10` throughout (same format as distance_sensor_config_t) — no unit conversion needed at integration point
- **D2-4**: QEMU simulation sweeps 0→25°C over 30s then resets — allows full gradient visual test in QEMU; follows existing sim pattern
- **D2-5**: Colour formula uses mid-point interpolation (no HSV/HSL conversion needed) — all integer, fits in ESP32 stack
- **D2-6**: Extension of `wifi_status_handler()` rather than new endpoint — temperature is "system status", not a separate resource. Consistent with REST intent.

### Horizontal Check (MECE)

- [x] SPEC_TEMP_ONEWIRE_1 covers all 1-Wire timing → SPEC_TEMP_TASK_1 covers sequencing → no overlap, no gap
- [x] SPEC_DSP_TEMP_1 explicitly guards against division-by-zero (config validation in REQ_CFG_JSON_16 AC-6 ensures cold < warm)
- [x] SPEC_WEB_TEMP_1 null handling aligns with REQ_WEB_TEMP_1 AC-3 (show "--°C" / return null)
- [x] Temperature API consumer chain complete: task → mutex → get_latest() → (web handler + display task + distance sensor)
- [x] QEMU coverage: SPEC_TEMP_ARCH_1 defines simulation strategy → no gap for SW-only testing

---

## Final Consistency Check

**Status**: 🔄 in progress

### 1. Traceability Verification

| Element | Links to | Status |
|---------|----------|--------|
| `US_TEMP_1` | — | ✅ root |
| `REQ_SYS_TEMP_1` | `US_TEMP_1` | ✅ |
| `REQ_TEMP_1` | `REQ_SYS_TEMP_1` | ✅ |
| `REQ_TEMP_2` | `REQ_TEMP_1` | ✅ |
| `REQ_TEMP_3` | `REQ_TEMP_2` | ✅ |
| `REQ_TEMP_4` | `REQ_TEMP_3` | ✅ |
| `REQ_TEMP_5` | `REQ_TEMP_4`, `REQ_SNS_15` | ✅ |
| `REQ_SNS_15` | `REQ_SNS_11` | ✅ |
| `REQ_DSP_6` | `REQ_DSP_3`, `REQ_TEMP_4`, `REQ_CFG_JSON_16` | ✅ |
| `REQ_WEB_TEMP_1` | `REQ_WEB_1`, `REQ_TEMP_4` | ✅ |
| `REQ_CFG_JSON_16` | `REQ_CFG_JSON_1`, `REQ_DSP_6`, `REQ_TEMP_1`, `REQ_TEMP_3` | ✅ |
| `SPEC_TEMP_ARCH_1` | `REQ_TEMP_1`, `REQ_TEMP_4` | ✅ |
| `SPEC_TEMP_ONEWIRE_1` | `REQ_TEMP_2` | ✅ |
| `SPEC_TEMP_TASK_1` | `REQ_TEMP_3`, `REQ_TEMP_5` | ✅ |
| `SPEC_TEMP_API_1` | `REQ_TEMP_1`, `REQ_TEMP_4` | ✅ |
| `SPEC_DSP_TEMP_1` | `REQ_DSP_6` | ✅ |
| `SPEC_WEB_TEMP_1` | `REQ_WEB_TEMP_1` | ✅ |

No orphaned elements.

### 2. Cross-Level Consistency

| US Intent | REQ Behaviour | SPEC Implementation |
|-----------|--------------|---------------------|
| Temperatur messen (Kompensation) | REQ_TEMP_1..5, REQ_SNS_15 | SPEC_TEMP_ARCH_1, SPEC_TEMP_ONEWIRE_1, SPEC_TEMP_TASK_1, SPEC_TEMP_API_1 |
| Temperatur im Web anzeigen | REQ_WEB_TEMP_1 | SPEC_WEB_TEMP_1 |
| LED-Farbe = Temperatur | REQ_DSP_6, REQ_CFG_JSON_16 | SPEC_DSP_TEMP_1, SPEC_DSP_ARCH_2 (modified) |
| Schwellenwerte konfigurierbar | REQ_CFG_JSON_16 | SPEC_CFG_JSON_EXTEND_1 (existing process) |
| QEMU-testbar | SPEC_TEMP_ARCH_1 (Simulation, 0→25°C sweep) | ✅ |

✅ No semantic drift detected.

### 3. MECE Across All Levels

- ✅ All 3 aspects of user request covered: Kompensation + Anzeige + Farbe
- ✅ Every REQ traces to a US, every SPEC traces to a REQ
- ✅ No gaps: DS18B20 lifecycle (init/start/stop/measure) fully covered

### 4. Document Completeness

- ✅ No DEPRECATED markers remain
- ✅ All 10 new requirement IDs defined with full RST content
- ✅ All modified requirements described in detail
- ✅ All 6 new spec IDs defined with full RST content
- ✅ All 3+ modified specs described

### 5. Files to Create / Modify

| Action | File |
|--------|------|
| **create** | `docs/11_requirements/req_temp_sensor.rst` |
| **create** | `docs/12_design/spec_temp_sensor.rst` |
| **modify** | `docs/11_requirements/req_system.rst` — add REQ_SYS_TEMP_1 |
| **modify** | `docs/11_requirements/req_distance_sensor.rst` — add REQ_SNS_15 |
| **modify** | `docs/11_requirements/req_display.rst` — modify REQ_DSP_3, modify REQ_DSP_2, add REQ_DSP_6 |
| **modify** | `docs/11_requirements/req_web_server.rst` — add REQ_WEB_TEMP_1 |
| **modify** | `docs/11_requirements/req_config_manager_json.rst` — add REQ_CFG_JSON_16 |
| **modify** | `docs/12_design/spec_display.rst` — modify SPEC_DSP_ALGO_1, modify SPEC_DSP_ARCH_2, add SPEC_DSP_TEMP_1 |
| **modify** | `docs/12_design/spec_web_server.rst` — modify SPEC_WEB_ROUTES_1, add SPEC_WEB_TEMP_1 |
| **modify** | `docs/11_requirements/index.rst` — add req_temp_sensor |
| **modify** | `docs/12_design/index.rst` — add spec_temp_sensor |
