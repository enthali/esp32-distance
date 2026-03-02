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

User Experience Requirements
-----------------------------

.. req:: Core Visualization Concept - Dual-Layer Display
   :id: REQ_DSP_3
   :links: REQ_SNS_11, REQ_DSP_1, REQ_DSP_6, REQ_DSP_7, REQ_DSP_8
   :status: approved
   :priority: mandatory
   :tags: display, ux, visualization

   **Description:** The display system SHALL use a dual-layer rendering architecture with position 
   indicator (upper layer) and zone-based background animations (lower layer) to provide clear 
   directional guidance and visual feedback of current vehicle position relative to ideal parking zone.

   **Rationale:** Original single-LED display showed only "where you are" but not "where you should be" 
   or "which direction to move". Dual-layer system addresses this by combining position tracking (white LED) 
   with zone-based animations that guide drivers forward/backward toward the ideal parking position. 
   The distance sensor component owns all measurement processing (REQ_SNS_11); display system provides 
   visual representation with enhanced user experience through layered rendering.

   **Acceptance Criteria:**

   - AC-1: Position indicator (white LED) renders on top of zone background animations
   - AC-2: LED position corresponds to processed distance measurement from sensor component
   - AC-3: Display updates immediately when new measurements arrive from sensor queue
   - AC-4: Display system SHALL NOT apply additional smoothing or filtering to distance measurements
   - AC-5: Zone-based background provides directional guidance (move forward/backward)
   - AC-6: Ideal parking zone is visually distinct and always identifiable

.. req:: Below Minimum Distance Warning (Superseded by REQ_DSP_7)
   :id: REQ_DSP_4
   :links: REQ_DSP_2, REQ_DSP_3, REQ_DSP_7
   :status: superseded
   :priority: mandatory
   :tags: display, ux, safety

   **Description:** [SUPERSEDED] The display system SHALL illuminate the first LED (LED 0) in red when the 
   measured distance is below the configured minimum distance threshold.

   **Rationale:** Original requirement provided basic warning. Superseded by zone-based system (REQ_DSP_7) 
   which provides enhanced emergency zone visualization with blinking red LEDs for urgent warning.

   **Superseded By:** REQ_DSP_7 (Zone 0 - Emergency zone with blinking red animation)

.. req:: Out of Range Display (Superseded by REQ_DSP_7)
   :id: REQ_DSP_5
   :links: REQ_DSP_2, REQ_DSP_4, REQ_DSP_7
   :status: superseded
   :priority: mandatory
   :tags: display, ux, error-handling

   **Description:** [SUPERSEDED] The display SHALL illuminate the last LED (LED led_count-1) in red when the 
   measured distance exceeds the configured maximum distance threshold, indicating out-of-range condition.

   **Rationale:** Original requirement provided basic out-of-range indication. Superseded by zone-based 
   system (REQ_DSP_7) which provides enhanced out-of-range visualization with blue indicator and ideal 
   zone reference.

   **Superseded By:** REQ_DSP_7 (Zone 4 - Out of range zone with informational display)

.. req:: Dual-Layer Rendering Architecture
   :id: REQ_DSP_6
   :links: REQ_DSP_3, REQ_DSP_1
   :status: approved
   :priority: mandatory
   :tags: display, architecture, rendering

   **Description:** The display system SHALL implement a dual-layer rendering architecture with lower 
   layer (zone-based background animations) and upper layer (position indicator) that composites into 
   a single frame before updating physical LEDs.

   **Rationale:** Dual-layer architecture separates concerns between directional guidance (background) 
   and position tracking (foreground), enabling clear visual hierarchy and smooth animations without 
   interference. Upper layer overwrites lower layer at position indicator LED for correct visual composition.

   **Acceptance Criteria:**

   - AC-1: Lower layer renders zone-based background pattern first
   - AC-2: Upper layer renders position indicator (white LED) on top
   - AC-3: Position LED color overwrites background color at that index
   - AC-4: Single led_show() call updates all LEDs atomically
   - AC-5: Rendering order is deterministic and consistent

.. req:: Zone-Based Display System
   :id: REQ_DSP_7
   :links: REQ_DSP_3, REQ_DSP_6, REQ_DSP_10
   :status: approved
   :priority: mandatory
   :tags: display, zones, ux

   **Description:** The display system SHALL divide the LED strip into 5 distinct zones based on 
   percentage distribution and provide zone-specific visual patterns that indicate vehicle position 
   relative to ideal parking zone and required directional movement.

   **Rationale:** Zone-based system provides intuitive parking guidance through color-coded animations. 
   Emergency zone (Zone 0) provides urgent warning with blinking red. Too-close zone (Zone 1) shows 
   orange caution with backward guidance. Ideal zone (Zone 2) provides clear stop indicator. Too-far 
   zone (Zone 3) shows forward guidance. Out-of-range zone (Zone 4) indicates sensor limits.

   **Zone Definitions:**

   - Zone 0 "Emergency": measurement < dist_min_mm (invalid, danger zone)
   - Zone 1 "Too Close": 0-20% of LED strip (back up required)
   - Zone 2 "Ideal": 20-40% of LED strip (perfect parking position)
   - Zone 3 "Too Far": 40-100% of LED strip (move forward)
   - Zone 4 "Out of Range": measurement > dist_max_mm (invalid, too far)

   **Acceptance Criteria:**

   - AC-1: Zone boundaries calculated using integer arithmetic with LED count percentage
   - AC-2: Zone 0 (Emergency): All Zone 1 LEDs blink red at 1 Hz, no position indicator
   - AC-3: Zone 1 (Too Close): Orange background, black LEDs animate toward ideal zone
   - AC-4: Zone 2 (Ideal): Bright red at ideal position, all else off, white position indicator
   - AC-5: Zone 3 (Too Far): Dim green LEDs animate toward ideal zone, white position indicator
   - AC-6: Zone 4 (Out of Range): Last LED at 5% blue, ideal zone at 5% green, no position indicator
   - AC-7: Zone calculations remain consistent across different LED counts (1-100 LEDs)

.. req:: Position Indicator Layer
   :id: REQ_DSP_8
   :links: REQ_DSP_6, REQ_DSP_3
   :status: approved
   :priority: mandatory
   :tags: display, position, layer

   **Description:** The display system SHALL render a single white LED at the calculated position 
   (upper layer) that overwrites the background animation color, visible only when measurements 
   are valid (Zones 1, 2, 3).

   **Rationale:** White LED provides clear position tracking distinct from colored zone backgrounds. 
   Hidden in invalid measurement zones (0, 4) to avoid showing misleading position information. 
   Color choice (white) ensures visibility against all background colors.

   **Acceptance Criteria:**

   - AC-1: Position LED color is pure white (RGB 255, 255, 255)
   - AC-2: Position LED visible in Zone 1, Zone 2, Zone 3
   - AC-3: Position LED hidden in Zone 0 and Zone 4 (invalid measurements)
   - AC-4: Position calculated using existing distance-to-LED mapping algorithm
   - AC-5: Position LED renders after background layer (overwrites background color)

.. req:: Animation Layer
   :id: REQ_DSP_9
   :links: REQ_DSP_6, REQ_DSP_7
   :status: approved
   :priority: mandatory
   :tags: display, animation, guidance

   **Description:** The display system SHALL implement smooth animations on the background layer 
   at approximately 100ms intervals to provide directional guidance toward the ideal parking zone.

   **Rationale:** Moving LED animations create intuitive directional cues. Blinking red in emergency 
   zone provides urgent warning. Moving black/green LEDs in Zone 1/3 guide driver toward ideal position. 
   Animation timing (100ms) provides smooth motion without excessive CPU overhead or visual flicker.

   **Acceptance Criteria:**

   - AC-1: Animation updates occur at approximately 100ms intervals (10 Hz)
   - AC-2: Animation state persists between frames for smooth motion
   - AC-3: Zone 0 blink animation: 1 Hz frequency (500ms on, 500ms off)
   - AC-4: Zone 1 animation: Two black LEDs move toward ideal zone continuously
   - AC-5: Zone 3 animation: Two green LEDs (5% brightness) move toward ideal zone continuously
   - AC-6: Zone 2 animation: Static display (no animation, bright red at ideal position)
   - AC-7: Zone 4 animation: Static display (no animation, dim blue and green indicators)

.. req:: Ideal Zone Visualization
   :id: REQ_DSP_10
   :links: REQ_DSP_7, REQ_DSP_3
   :status: approved
   :priority: mandatory
   :tags: display, ideal-zone, reference

   **Description:** The display system SHALL provide a consistent visual reference to the ideal 
   parking zone (center of Zone 2, approximately 30% of LED strip) across all zone states to 
   help drivers understand the target position.

   **Rationale:** Ideal zone reference ensures drivers always know "where to stop" regardless of 
   current vehicle position. In Zone 2, bright red provides clear "STOP HERE" signal. In other zones, 
   subtle ideal zone indicator (5% brightness) serves as reference point without overwhelming 
   directional guidance animations.

   **Acceptance Criteria:**

   - AC-1: Ideal zone LED calculated as 30% of LED strip (center of Zone 2)
   - AC-2: Zone 0: Ideal LED at 5% red (subtle reference during emergency)
   - AC-3: Zone 1: Ideal LED at 5% red (target reference while backing up)
   - AC-4: Zone 2: Ideal LED at 100% red (bright "STOP HERE" signal)
   - AC-5: Zone 3: Ideal LED at 5% green (target reference while moving forward)
   - AC-6: Zone 4: Ideal LED at 5% green (distance reference when out of range)
   - AC-7: Ideal zone position remains constant regardless of vehicle position

Traceability
------------

.. needtable::
   :filter: "REQ_DSP" in id
   :columns: id, title, status, links

.. needflow::
   :filter: "REQ_DSP" in id
   :types: req
