Display System Design Specification
======================================

This document specifies the design of the display system that visualizes distance measurements 
through WS2812 LED strips with dual-layer rendering and zone-based animations.

**Document Version**: 2.0  
**Last Updated**: 2025-11-12

Overview
--------

The display system implements a multi-layer rendering architecture combining position indication 
and directional animations to guide optimal parking, using frame-based compositing at 10 FPS.

System Integration
------------------

.. spec:: WS2812 Hardware Integration
   :id: SPEC_DSP_OVERVIEW_1
   :links: REQ_DSP_1
   :status: approved
   :tags: display, hardware, integration

   **Design:** WS2812 addressable LED strip as primary display hardware with config-driven 
   parameters.

   **Implementation:**

   - LED count: Configurable via ``config_manager`` API (1-100 LEDs, validated)
   - Brightness: Configurable via ``config_manager`` API (0-255, hardware PWM control)
   - GPIO pin: Hardware-specific configuration for WS2812 data line
   - Initialization: LED hardware initialized before task starts processing measurements
   - Real-time operation: Continuous reactive updates as distance measurements arrive

   **Validation:** LED strip responds to configuration changes, hardware initialization successful.

.. spec:: Task-Based Architecture
   :id: SPEC_DSP_ARCH_1
   :links: REQ_DSP_12
   :status: approved
   :tags: display, freertos, task

   **Design:** Implement single FreeRTOS task that blocks on ``distance_sensor_get_latest()``.

   **Implementation:**

   - Task priority set below measurement task to ensure proper data flow hierarchy
   - Task runs continuously until system restart
   - Core assignment and stack size from centralized configuration
   - Blocking wait pattern eliminates polling overhead and provides immediate response

   **Validation:** Task created successfully, priority hierarchy maintained, blocks efficiently.

.. spec:: Configuration Integration
   :id: SPEC_DSP_ARCH_2
   :links: REQ_DSP_2, REQ_CFG_JSON_1
   :status: approved
   :tags: display, configuration

   **Design:** Use ``config_manager`` API for all distance range parameters.

   **Implementation:**

   - Obtain min/max distance values via ``config_get_current()`` and use ``distance_min_mm`` / 
     ``distance_max_mm`` fields (millimeters)
   - Cache config values locally at task startup for performance
   - Configuration changes handled via system restart (restart-based architecture)
   - Configuration validation responsibility belongs to ``config_manager``

   **Validation:** All distance parameters obtained from ``config_manager`` API, no separate 
   config structures.

Core Algorithms
---------------

.. spec:: Distance-to-Visual Mapping Algorithm
   :id: SPEC_DSP_ALGO_1
   :links: REQ_DSP_14, REQ_DSP_3, REQ_DSP_4
   :status: approved
   :tags: display, algorithm, mapping

   **Design:** Distance-to-LED mapping with zone-based visual behavior.

   **Zone Calculation** (configuration-independent, integer math):

   - ``ideal_size = (led_count * 10) / 100`` (10% of strip)
   - ``ideal_center = (led_count * 30) / 100`` (30% position)
   - ``ideal_start = ideal_center - (ideal_size / 2)``
   - ``ideal_end = ideal_start + ideal_size - 1``
   - Example for 40 LEDs: ideal_center=12, ideal_size=4, ideal_start=10, ideal_end=13

   **Position Mapping:**

   - Formula: ``led_index = (distance_mm - min_mm) * (led_count - 1) / (max_mm - min_mm)``
   - Boundary clamping: ``[0, led_count-1]``

   **Zone Behaviors:**

   - Zone 1 "too close" (led_index < ideal_start): Orange position (50%) + red animation (100%)
   - Zone 2 "ideal" (ideal_start ≤ led_index ≤ ideal_end): All ideal zone LEDs red (100%)
   - Zone 3 "too far" (led_index > ideal_end): Green position (100%) + white animation (2%)
   - Below minimum: Emergency blinking pattern
   - Above maximum: White animation only (no position indicator)

   **Validation:** Zone boundaries calculated correctly, position mapping linear, animations 
   trigger in correct zones.

.. spec:: Multi-Layer Rendering Pipeline
   :id: SPEC_DSP_ALGO_2
   :links: REQ_DSP_13, REQ_DSP_10
   :status: approved
   :tags: display, rendering, architecture

   **Design:** Frame-based rendering with priority-based layer compositing at 10 FPS.

   **Rendering Pipeline** (executed at 100ms intervals):

   1. **Clear**: ``led_clear_all()`` - reset all LEDs to off state
   2. **Ideal Zone Background Layer** (2% red brightness):
      - Always render ideal zone LEDs (ideal_start through ideal_end) at 2% brightness
      - Provides constant visual reference of target parking zone
   3. **Animation Layer** (2-100% brightness):
      - Render white or red animation LED at current animation position
   4. **Position Layer** (50-100% brightness):
      - Render orange or green LED at measured position (overwrites animation if same position)
   5. **Ideal Zone Full Brightness Layer**:
      - If in ideal zone: Overwrite ALL ideal zone LEDs with red at 100% brightness
   6. **Emergency Layer** (highest priority):
      - If below minimum: Overwrite LEDs 0, 10, 20, 30... with blinking red
   7. **Commit**: ``led_show()`` - transmit complete buffer atomically

   **Timing:**

   - FreeRTOS timer callback at 100ms intervals for animation updates
   - Distance measurement updates position immediately (no waiting for timer)
   - Non-blocking: Timer callback completes in <1ms

   **Validation:** All layers composite correctly, priority enforced, single led_show() per 
   frame, no visual tearing, animation smooth at 10 FPS.

.. spec:: Embedded Arithmetic Architecture
   :id: SPEC_DSP_ALGO_3
   :links: REQ_SYS_1
   :status: approved
   :tags: display, performance, arithmetic

   **Design:** Pure integer arithmetic for all distance calculations and display operations.

   **Implementation:**

   - Distance representation: ``uint16_t`` millimeters (0-65535mm)
   - Position calculations: Multiplication before division for precision preservation
   - Boundary checks: Integer comparisons
   - Memory efficiency: 2-byte integers vs 4-byte floats
   - Execution speed: Integer ALU operations vs FPU operations
   - Deterministic timing: No floating-point precision variations

   **Rationale:** Avoid floating-point on resource-constrained microcontrollers unless necessary.

   **Validation:** All arithmetic operations complete within deterministic time bounds.

Animation Specifications
------------------------

.. spec:: Directional Animation Design
   :id: SPEC_DSP_ANIM_1
   :links: REQ_DSP_7, REQ_DSP_8, REQ_DSP_10
   :status: approved
   :tags: display, animation, guidance

   **Design:** Running LED animation for directional guidance.

   **Animation State Machine:**

   .. code-block:: c

      typedef struct {
          uint8_t current_position;    // Current animation LED position
          bool animation_active;       // Is animation running?
          bool animation_direction;    // true = forward, false = backward
          uint8_t animation_start;     // Start position
          uint8_t animation_end;       // End position
      } animation_state_t;

   **Animation Behaviors:**

   - **Too Far Zone** (position > ideal_end):
     - Start: ``led_count - 1`` (far end), End: ``ideal_end``
     - Direction: Backward (toward ideal zone)
     - Color: White at 2% brightness (~5, ~5, ~5)
     - Loop: When reaching ideal_end, restart from led_count-1

   - **Too Close Zone** (position < ideal_start):
     - Start: 0 (near end), End: ``ideal_start``
     - Direction: Forward (toward ideal zone)
     - Color: Red at 100% brightness (255, 0, 0)
     - Loop: When reaching ideal_start, restart from 0

   - **Animation Update** (100ms timer):
     - Increment/decrement position based on direction
     - Wrap around when reaching end position
     - Independent of distance measurement updates

   **Validation:** Animation smooth, loops correctly, stops when zone changes, color brightness 
   correct.

.. spec:: Ideal Zone Display Design
   :id: SPEC_DSP_ANIM_2
   :links: REQ_DSP_9, REQ_DSP_10
   :status: approved
   :tags: display, ideal-zone, feedback

   **Design:** Dual-brightness red indication for ideal parking zone providing constant visual 
   reference and positive feedback.

   **Ideal Zone Background Rendering** (always active):

   - Loop through LEDs from ideal_start to ideal_end
   - Set each LED to red at 2% brightness (~5, 0, 0)
   - Renders as base layer (lowest priority)
   - Provides constant visual target reference visible from any distance

   **Ideal Zone Full Brightness Rendering** (when in zone):

   - When ``ideal_start ≤ led_index ≤ ideal_end``:
     - Set all LEDs (ideal_start to ideal_end) to solid red (255, 0, 0)
     - Overwrite any animation or position layer LEDs
     - No animation or position indicator (entire zone valid)

   **Validation:** Background always visible at 2%, full brightness overrides other layers, 
   immediate transition provides clear feedback.

.. spec:: Emergency Blinking Pattern Design
   :id: SPEC_DSP_ANIM_3
   :links: REQ_DSP_11, REQ_DSP_5
   :status: approved
   :tags: display, safety, emergency

   **Design:** 1 Hz blinking pattern on every 10th LED for emergency warning.

   **Blink State Machine:**

   .. code-block:: c

      typedef struct {
          bool blink_state;           // true = ON, false = OFF
          uint32_t last_toggle_ms;    // Last toggle timestamp
      } blink_state_t;

   **Blink Behavior:**

   - **Timing**: Toggle every 500ms using FreeRTOS tick count
   - **ON State**: LEDs at positions 0, 10, 20, 30, ... show red (255, 0, 0)
   - **OFF State**: LEDs at positions 0, 10, 20, 30, ... turn off (0, 0, 0)
   - **Priority**: Highest priority, overrides all other layers
   - **Suppression**: Ideal zone background and all other layers suppressed during emergency

   **Validation:** Blink frequency accurate at 1 Hz, distributed pattern visible from all 
   angles, emergency overrides all other displays.

.. spec:: Layer Priority Architecture
   :id: SPEC_DSP_ANIM_4
   :links: REQ_DSP_10
   :status: approved
   :tags: display, rendering, priority

   **Design:** Deterministic layer priority with proper compositing.

   **Layer Priority** (lowest to highest):

   1. **Base Layer**: Ideal zone background (2% red) - always visible
   2. **Animation Layer**: White (too far) or red (too close) directional animation
   3. **Position Layer**: Orange (too close) or green (too far) position indicator
   4. **Ideal Zone Override**: Full brightness ideal zone (100% red) when in zone
   5. **Emergency Override**: Blinking red pattern (highest priority)

   **Compositing Rules:**

   - Higher priority layer overwrites lower priority at same LED position
   - Single ``led_show()`` call per frame ensures atomic updates
   - No partial frame updates or tearing
   - Frame rate: 10 FPS (100ms per frame)

   **Validation:** Priority enforced correctly, no visual artifacts, smooth transitions between 
   states.

API Design
----------

.. spec:: Public Display API
   :id: SPEC_DSP_API_1
   :links: REQ_DSP_12
   :status: approved
   :tags: display, api, interface

   **Design:** Minimal public API for display system initialization and task management.

   **Public Functions:**

   - ``display_init()``: Initialize display system with configuration
   - ``display_start()``: Create and start display task
   - ``display_stop()``: Stop display task gracefully

   **Internal Functions:**

   - ``display_task()``: Main task loop (blocking on distance measurements)
   - ``animation_timer_callback()``: 100ms timer for animation updates
   - ``calculate_led_position()``: Distance-to-LED mapping
   - ``render_frame()``: Multi-layer rendering pipeline

   **Validation:** API minimal and clear, internal functions properly encapsulated, task 
   lifecycle managed correctly.
