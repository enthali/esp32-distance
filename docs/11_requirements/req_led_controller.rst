LED Controller Requirements
============================

This document specifies requirements for WS2812 LED strip control, focusing on user-facing 
functionality and hardware interface.

**Document Version**: 1.0  
**Last Updated**: 2025-11-10

.. contents:: Table of Contents
   :local:
   :depth: 2

Overview
--------

The LED controller component provides precise control of WS2812 addressable LED strips for 
visual distance display, supporting individual pixel control and configurable strip lengths.

Functional Requirements
-----------------------

.. req:: WS2812 LED Strip Support
   :id: REQ_LED_1
   :links: REQ_SYS_1
   :status: approved
   :priority: mandatory
   :tags: led, hardware, ws2812

   **Description:** The system SHALL support WS2812 addressable LED strips for visual distance 
   display, providing hardware compatibility and initialization.

   **Rationale:** WS2812 LEDs are the chosen hardware for distance visualization; system must 
   interface with this specific LED technology.

   **Acceptance Criteria:**

   - AC-1: System SHALL initialize WS2812 LED strips connected to configurable GPIO pin
   - AC-2: System SHALL support WS2812 timing and protocol requirements
   - AC-3: LEDs SHALL respond to control signals and display colors
   - AC-4: Initialization SHALL handle hardware configuration parameters

   **Verification:** Connect WS2812 strip, verify LEDs respond to basic commands and display 
   expected colors.

.. req:: Individual Pixel Control
   :id: REQ_LED_2
   :links: REQ_LED_1
   :status: approved
   :priority: mandatory
   :tags: led, control, api

   **Description:** The system SHALL provide the ability to set the color of individual LED 
   pixels within the strip for precise distance visualization.

   **Rationale:** Distance display requires lighting specific LEDs at calculated positions; 
   individual control is essential for the application.

   **Acceptance Criteria:**

   - AC-1: System SHALL set specific LED colors by index position
   - AC-2: Color control SHALL support RGB color space with 8-bit resolution per channel
   - AC-3: Individual LED changes SHALL NOT affect other LEDs
   - AC-4: System SHALL provide clear all LEDs functionality

   **Verification:** Set individual LEDs to different colors, verify only targeted LEDs change, 
   test clear all functionality.

.. req:: Configurable LED Count
   :id: REQ_LED_3
   :links: REQ_LED_1, REQ_CFG_1
   :status: approved
   :priority: mandatory
   :tags: led, configuration, flexibility

   **Description:** The system SHALL support configurable LED strip lengths to accommodate 
   different hardware deployments and installation requirements. LED count is a user-configurable 
   parameter managed by the configuration system (REQ_CFG_1).

   **Rationale:** Different installations may use varying LED strip lengths; system must be 
   flexible for deployment scenarios. LED count is a user-facing parameter that affects system 
   behavior and visual representation, therefore it belongs in centralized configuration management.

   **Acceptance Criteria:**

   - AC-1: LED count SHALL be obtained from centralized configuration system (REQ_CFG_1)
   - AC-2: LED controller SHALL receive LED count during initialization via configuration structure
   - AC-3: System SHALL support LED counts from 1 to at least 100 LEDs
   - AC-4: Configuration system SHALL validate LED count parameters (validation owned by REQ_CFG_6)
   - AC-5: All LED positions within configured count SHALL be controllable

   **Verification:** Test with different LED counts, verify all configured LEDs are controllable 
   and out-of-range access is prevented.

.. req:: Accurate Color Display
   :id: REQ_LED_4
   :links: REQ_LED_2
   :status: approved
   :priority: mandatory
   :tags: led, quality, color-accuracy

   **Description:** The system SHALL display specified RGB colors accurately on WS2812 LEDs for 
   reliable visual feedback to users.

   **Rationale:** Color accuracy is essential for distance visualization effectiveness; users 
   must be able to distinguish between different states (normal/error conditions).

   **Acceptance Criteria:**

   - AC-1: Primary colors (red, green, blue) SHALL display distinctly and accurately
   - AC-2: White color SHALL appear as true white without color bias
   - AC-3: Black/off state SHALL turn LEDs completely off
   - AC-4: Color transitions SHALL be immediate when hardware is updated

   **Verification:** Display primary colors and verify visual accuracy, test white balance and 
   off state, measure color transition timing.

.. req:: LED State Read API
   :id: REQ_LED_5
   :links: REQ_LED_2, REQ_WEB_1
   :status: approved
   :priority: important
   :tags: led, api, monitoring, web-interface

   **Description:** The system SHALL provide a thread-safe API to read the current state of all 
   LED colors for monitoring and visualization purposes (e.g., web interface LED display).

   **Rationale:** External components (web server, monitoring tasks) need to read LED state for 
   real-time visualization without interfering with LED control operations. Thread-safe access 
   is critical in multi-task RTOS environment.

   **Acceptance Criteria:**

   - AC-1: System SHALL provide ``led_get_all_colors()`` function to retrieve snapshot of all LED colors
   - AC-2: Function SHALL return stable snapshot matching last physical LED update (after ``led_show()``)
   - AC-3: Function SHALL be thread-safe using mutex protection
   - AC-4: Function SHALL support partial reads limited by buffer size
   - AC-5: Function SHALL return actual number of LEDs copied or 0 on error
   - AC-6: Caller SHALL provide pre-allocated buffer of sufficient size

   **Verification:** Call from multiple tasks concurrently, verify no race conditions, confirm 
   returned colors match physical LED state, test with various buffer sizes.
