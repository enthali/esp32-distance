Display System Requirements
=============================

This document specifies requirements for the Display System, enabling visual representation 
of distance measurements through LED strip control with clear user experience definitions.

**Document Version**: 2.0  
**Last Updated**: 2025-11-12

Overview
--------

The display system provides visual feedback through WS2812 LED strips, showing current distance 
measurement with dual-layer architecture combining position indication and directional animation 
to guide optimal parking.

System Overview Requirements
-----------------------------

.. req:: Hardware Platform
   :id: REQ_DSP_1
   :links: REQ_LED_1, REQ_SYS_1
   :status: approved
   :priority: mandatory
   :tags: display, hardware, integration

   **Description:** The display system SHALL utilize WS2812 addressable LED strip hardware through 
   the LED controller component API as an integrated, reactive component. LED count and brightness 
   are configurable via configuration management system.

   **Rationale:** Establishes the hardware foundation for the visual display system, enabling 
   flexible LED strip configurations for different deployment scenarios. The system operates 
   reactively, automatically responding to distance measurements rather than external commands. 
   Display system uses LED controller API for all hardware access, maintaining clear separation 
   of concerns.

   **Acceptance Criteria:**

   - AC-1: Display system uses LED controller component API for all LED hardware access
   - AC-2: LED count and brightness are configurable via configuration management system
   - AC-3: System validates LED count is within reasonable range (1-100 LEDs)
   - AC-4: LED controller component handles all GPIO pin configuration and timing
   - AC-5: Display system depends on LED controller initialization completing successfully
   - AC-6: System operates continuously, updating display in real-time as measurements arrive

.. req:: Configuration Integration
   :id: REQ_DSP_2
   :links: REQ_CFG_JSON_1, REQ_SYS_CFG_1
   :status: approved
   :priority: mandatory
   :tags: display, configuration, integration

   **Description:** The display system SHALL obtain all operational parameters from the 
   configuration management system.

   **Rationale:** Ensures centralized configuration management and eliminates hardcoded values, 
   enabling runtime reconfiguration and consistent system behavior. Parameter validation is 
   handled by the configuration manager. Configuration changes trigger system reset, allowing 
   static memory allocation at startup with no runtime reallocation - preventing memory 
   fragmentation and performance degradation in the embedded environment.

   **Acceptance Criteria:**

   - AC-1: All display parameters are obtained from configuration manager
   - AC-2: System reads configuration at initialization

   **Configuration Parameters:**

   - ``led_count``: Number of LEDs in the strip
   - ``led_brightness``: LED brightness level
   - ``dist_min_mm``: Minimum distance threshold (millimeters)
   - ``dist_max_mm``: Maximum distance threshold (millimeters)
   - ``temp_cold_c10``: Cold temperature threshold (tenths of °C, default 50 = 5.0°C)
   - ``temp_warm_c10``: Warm temperature threshold (tenths of °C, default 200 = 20.0°C)

User Experience Requirements
-----------------------------

.. req:: Core Visualization Concept
   :id: REQ_DSP_3
   :links: REQ_SNS_4, REQ_DSP_1
   :status: approved
   :priority: mandatory
   :tags: display, ux, visualization

   **Description:** The display system SHALL illuminate a single LED in a colour determined by the
   current temperature (per REQ_DSP_6) that represents the current measured distance with
   real-time updates. The display system consumes processed distance measurements from the distance
   sensor component without additional filtering or smoothing.

   **Rationale:** Provides clear, unambiguous visual feedback where users can immediately
   understand the current distance measurement through LED position. The distance sensor component
   owns all measurement processing, filtering, and smoothing (REQ_SNS_11); the display system is
   responsible solely for visual representation of processed measurements, maintaining clear
   separation of concerns. Temperature dual-coding adds ambient context without extra hardware.

   **Acceptance Criteria:**

   - AC-1: Only one LED is illuminated at any given time
   - AC-2: LED position corresponds to processed distance measurement value from sensor component
   - AC-3: Display updates immediately when new processed measurements arrive from sensor queue
   - AC-4: Display system SHALL NOT apply additional smoothing or filtering to distance measurements
   - AC-5: Display system uses distance measurements directly from sensor component's processed measurement queue
   - AC-6: LED colour for the position indicator is determined by REQ_DSP_6 (temperature-based gradient)

.. req:: Below Minimum Distance Warning
   :id: REQ_DSP_4
   :links: REQ_DSP_2, REQ_DSP_3
   :status: approved
   :priority: mandatory
   :tags: display, ux, safety

   **Description:** The display system SHALL illuminate the first LED (LED 0) in red when the 
   measured distance is below the configured minimum distance threshold.

   **Rationale:** Red is universally recognized as a warning color. Illuminating the first LED 
   in red when too close provides immediate, unmistakable indication of dangerous proximity.

   **Acceptance Criteria:**

   - AC-1: LED 0 is illuminated red when distance < dist_min_mm
   - AC-2: No green position indicator is shown (measurement invalid)
   - AC-3: Display persists until measurement returns to valid range

.. req:: Out of Range Display
   :id: REQ_DSP_5
   :links: REQ_DSP_2, REQ_DSP_4
   :status: approved
   :priority: mandatory
   :tags: display, ux, error-handling

   **Description:** the display SHALL iluminate the last LED (LED led_count-1) in red when the 
   measured distance exceeds the configured maximum distance threshold, indicating out-of-range condition.

   **Rationale:** Red is universally recognized as an error/warning color. Illuminating the last 
   LED in red when too far provides clear indication that the measurement is invalid and out of range.

   **Acceptance Criteria:**

   - AC-1: last LED (LED led_count-1) is illuminated red when distance > dist_max_mm
   - AC-2: No green position indicator is shown (measurement invalid)
   - AC-3: Display persists until measurement returns to valid range

.. req:: Temperature-Based Position LED Colour
   :id: REQ_DSP_6
   :status: approved
   :priority: mandatory
   :links: REQ_DSP_3, REQ_TEMP_4, REQ_CFG_JSON_16
   :tags: display, temperature, colour, ux

   **Description:** The display system SHALL apply a temperature-dependent colour to the position
   indicator LED using a blue→green→orange gradient, with configurable cold and warm temperature
   thresholds.

   **Rationale:** The LED position already encodes distance; using colour to encode temperature
   provides a second channel of information without adding hardware. Blue for cold (< 5°C) is a
   universally understood safety signal for icy garage conditions. The gradient gives a continuous,
   intuitive readout.

   **Acceptance Criteria:**

   - AC-1: At or below ``temp_cold_c10`` threshold the position LED SHALL be blue (0, 0, 255)
   - AC-2: At or above ``temp_warm_c10`` threshold the position LED SHALL be orange (255, 165, 0)
   - AC-3: Between thresholds the colour SHALL be a linear interpolation between blue and orange via green
   - AC-4: Both thresholds SHALL be read from the configuration system (keys ``temp_cold_c10`` and ``temp_warm_c10``)
   - AC-5: If no valid temperature reading is available, the position LED SHALL default to green (original behaviour maintained)
   - AC-6: Colour mapping SHALL NOT affect the warning colour logic of REQ_DSP_4 (too close → red) or REQ_DSP_5 (out of range → red)


Traceability
------------

.. needtable::
   :filter: "REQ_DSP" in id
   :columns: id, title, status, links

.. needflow::
   :filter: "REQ_DSP" in id
   :types: req
