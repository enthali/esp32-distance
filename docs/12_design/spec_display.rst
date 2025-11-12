Display System Design Specification
======================================

This document specifies the design of the display system that visualizes distance measurements 
through WS2812 LED strips with dual-layer rendering and zone-based animations.

**Document Version**: 2.0  
**Last Updated**: 2025-11-12

Overview
--------

The display system implements straight forward distance-to-LED mapping

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
   :links: REQ_DSP_3
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

   - Obtain min/max distance values via ``config_get_int32()`` using ``dist_min_mm`` / 
     ``dist_max_mm`` fields (millimeters)
   - Cache config values locally at task startup for performance
   - Configuration changes handled via system restart (restart-based architecture)
   - Configuration validation responsibility belongs to ``config_manager``

   **Validation:** All distance parameters obtained from ``config_manager`` API, no separate 
   config structures.

Core Algorithms
---------------

.. spec:: Distance-to-Visual Mapping Algorithm
   :id: SPEC_DSP_ALGO_1
   :links: REQ_DSP_3, REQ_DSP_4, REQ_DSP_5
   :status: approved
   :tags: display, algorithm, mapping

   **Design:** Distance-to-LED mapping with zone-based guidance system.

   **Position Mapping:**

   - Formula: ``led_index = (distance_mm - min_mm) * (led_count - 1) / (max_mm - min_mm)``
   - Boundary clamping: ``[0, led_count-1]``

   **Zone Boundaries (Integer Math):**

   - Zone 1 end: ``(led_count * 20) / 100`` (20% of strip)
   - Zone 2 end: ``(led_count * 40) / 100`` (40% of strip)
   - Zone 3: From zone 2 end to ``led_count - 1``

   **Zone Detection:**

   - Zone 0: ``distance_mm < dist_min_mm``
   - Zone 1: ``led_index >= 0 && led_index < zone1_end``
   - Zone 2: ``led_index >= zone1_end && led_index < zone2_end``
   - Zone 3: ``led_index >= zone2_end && led_index < led_count``
   - Zone 4: ``distance_mm > dist_max_mm``

   **Validation:** Position updates correctly with distance changes, zone boundaries calculated accurately.

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

.. spec:: Animation State Machine Design
   :id: SPEC_DSP_ALGO_4
   :links: REQ_DSP_8
   :status: draft
   :tags: display, animation, state-machine

   **Design:** Stack-allocated animation state machine with frame-based timing.

   **State Structure:**

   .. code-block:: c

      typedef struct {
          uint32_t frame_counter;      // Total frames rendered since start
          uint32_t last_update_ms;     // Last animation update timestamp
          bool blink_state;            // Zone 0 emergency blink state
      } animation_state_t;

   **Timing Logic:**

   - Check elapsed time: ``current_ms - last_update_ms``
   - Update if ``>= 100ms`` for normal animation frames
   - Update if ``>= 500ms`` for Zone 0 blink toggle
   - Use ``esp_timer_get_time() / 1000`` for millisecond timestamps

   **Frame Updates:**

   - Increment ``frame_counter`` on each animation update
   - Use modulo arithmetic for cyclic patterns: ``frame % pattern_length``
   - Moving indicators: ``position = start + (frame % zone_length)``

   **Validation:** Animation timing accurate within ±20ms, no memory leaks, smooth visual transitions.

.. spec:: Dual-Layer Rendering Architecture
   :id: SPEC_DSP_RENDER_1
   :links: REQ_DSP_6, REQ_DSP_7
   :status: draft
   :tags: display, rendering, architecture

   **Design:** Two-pass rendering system with strict layering order.

   **Rendering Pipeline:**

   1. **Clear**: ``led_clear_all()`` - Reset all LEDs to black
   2. **Lower Layer**: Render zone backgrounds and animations
      - Zone 0: Blinking RED on Zone 1 LEDs
      - Zone 1: ORANGE background + moving BLACK indicators
      - Zone 2: Solid 100% RED
      - Zone 3: Moving GREEN indicators at 5%
      - Zone 4: BLUE indicator + GREEN ideal zone at 5%
   3. **Upper Layer**: Render WHITE position indicator (overwrites lower layer)
   4. **Show**: ``led_show()`` - Update physical LEDs

   **Color Specifications:**

   - WHITE (position): ``RGB(255, 255, 255)``
   - RED (100%): ``RGB(255, 0, 0)``
   - RED (5%): ``RGB(13, 0, 0)``
   - GREEN (5%): ``RGB(0, 13, 0)``
   - ORANGE: ``RGB(255, 165, 0)``
   - BLUE (5%): ``RGB(0, 0, 13)``
   - BLACK (moving): ``RGB(0, 0, 0)``

   **Helper Functions:**

   .. code-block:: c

      static led_color_t led_color_scale(led_color_t color, uint8_t percentage);
      static led_color_t led_color_rgb_custom(uint8_t r, uint8_t g, uint8_t b);

   **Validation:** Upper layer always visible, lower layer provides context, no rendering artifacts.

API Design
----------

.. spec:: Public Display API
   :id: SPEC_DSP_API_1
   :links: REQ_DSP_3
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
