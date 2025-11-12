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

.. spec:: Distance-to-Visual Mapping Algorithm with Zone Detection
   :id: SPEC_DSP_ALGO_1
   :links: REQ_DSP_3, REQ_DSP_7, REQ_DSP_8, SPEC_DSP_ZONES_1
   :status: approved
   :tags: display, algorithm, mapping

   **Design:** Distance-to-LED mapping with zone-based boundary detection for dual-layer rendering.

   **Position Mapping (Upper Layer):**

   - Formula: ``led_index = (distance_mm - min_mm) * (led_count - 1) / (max_mm - min_mm)``
   - Boundary clamping: ``[0, led_count-1]``
   - Color: White LED for position indicator (REQ_DSP_8)
   - Visibility: Only in Zones 1, 2, 3 (hidden in emergency/out-of-range zones)

   **Zone Detection:**

   - Zone 0 (Emergency): ``distance_mm < dist_min_mm``
   - Zone 1 (Too Close): ``0 <= led_index < (led_count * 20 / 100)``
   - Zone 2 (Ideal): ``(led_count * 20 / 100) <= led_index < (led_count * 40 / 100)``
   - Zone 3 (Too Far): ``(led_count * 40 / 100) <= led_index < led_count``
   - Zone 4 (Out of Range): ``distance_mm > dist_max_mm``

   **Behaviors:**

   - Dual-layer rendering: Background (zone pattern) + Foreground (white position LED)
   - Position LED overwrites background color at calculated index
   - Zone-specific background patterns provide directional guidance

   **Validation:** Position updates correctly with distance changes, zone transitions smooth.

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

.. spec:: Zone Calculation and Boundaries
   :id: SPEC_DSP_ZONES_1
   :links: REQ_DSP_7, REQ_DSP_10, SPEC_DSP_ALGO_3
   :status: approved
   :tags: display, zones, algorithm

   **Design:** Integer-based zone boundary calculations using LED strip percentage distribution 
   with ideal zone reference point for parking guidance.

   **Zone Boundary Calculations:**

   .. code-block:: c

      // Given: led_count, dist_min_mm, dist_max_mm
      uint16_t zone1_end = led_count * 20 / 100;  // 20% of LEDs
      uint16_t zone2_end = led_count * 40 / 100;  // 40% of LEDs
      uint16_t ideal_led = led_count * 30 / 100;  // 30% center of ideal zone

   **Zone Detection Logic:**

   .. code-block:: c

      // Zone determination from distance measurement
      if (distance_mm < dist_min_mm) {
          zone = 0;  // Emergency
      } else if (distance_mm > dist_max_mm) {
          zone = 4;  // Out of Range
      } else {
          // Calculate LED position
          led_index = (distance_mm - dist_min_mm) * (led_count - 1) / (dist_max_mm - dist_min_mm);
          
          if (led_index < zone1_end) zone = 1;       // Too Close
          else if (led_index < zone2_end) zone = 2;  // Ideal
          else zone = 3;                              // Too Far
      }

   **Ideal Zone Reference:**

   - Position: 30% of LED strip (center of Zone 2)
   - Purpose: Visual target for driver across all zones
   - Brightness: Variable (100% in Zone 2, 5% in other zones)
   - Color: Red in Zones 0-2, Green in Zones 3-4

   **Validation:** Zone boundaries stable across all valid LED counts (1-100), calculations 
   use integer math only, no floating point operations.

.. spec:: Dual-Layer Rendering Architecture
   :id: SPEC_DSP_LAYERS_1
   :links: REQ_DSP_6, REQ_DSP_8, REQ_DSP_9
   :status: approved
   :tags: display, architecture, rendering

   **Design:** Two-pass rendering pipeline with compositing for visual clarity and separation 
   of position tracking from directional guidance.

   **Rendering Pipeline:**

   .. code-block:: c

      void render_frame(zone, distance_mm) {
          // Pass 1: Clear buffer
          led_clear_all();
          
          // Pass 2: Render lower layer (zone background)
          render_zone_background(zone);
          
          // Pass 3: Render upper layer (position indicator)
          if (zone >= 1 && zone <= 3) {  // Valid measurement zones
              led_index = calculate_position(distance_mm);
              led_set_pixel(led_index, LED_COLOR_WHITE);  // Overwrites background
          }
          
          // Pass 4: Update physical LEDs
          led_show();
      }

   **Layer Composition:**

   - Lower layer (Background): Zone-based patterns (animations, colors)
   - Upper layer (Position): Single white LED at calculated position
   - Composition: Upper layer overwrites lower layer at position LED index
   - Update: Atomic update via single ``led_show()`` call

   **Memory Architecture:**

   - Single LED buffer (no double buffering)
   - In-place layer composition
   - Stack-based animation state (~16 bytes)
   - No heap allocation during rendering

   **Validation:** Rendering completes within 20ms frame budget, layers composite correctly, 
   position indicator always visible on top of background.

.. spec:: Animation Patterns and Timing
   :id: SPEC_DSP_ANIM_1
   :links: REQ_DSP_9, REQ_DSP_10, SPEC_DSP_ZONES_1
   :status: approved
   :tags: display, animation, timing

   **Design:** Timing-based animation system using ESP32 microsecond timer for smooth, 
   consistent motion without blocking or task delays.

   **Animation State Structure:**

   .. code-block:: c

      typedef struct {
          uint32_t animation_step;      // Current animation frame counter
          uint64_t last_update_time;    // Last animation update (microseconds)
          bool blink_state;             // Zone 0 blink state (on/off)
      } display_animation_state_t;

   **Timing Control:**

   - Animation update interval: 100ms (10 Hz frame rate)
   - Zone 0 blink period: 1000ms (1 Hz, 500ms on / 500ms off)
   - Timing source: ``esp_timer_get_time()`` (microsecond precision)
   - Update trigger: Check elapsed time on each distance measurement arrival

   **Zone-Specific Patterns:**

   **Zone 0 (Emergency):**
   
   - Pattern: All Zone 1 LEDs blink red at 1 Hz
   - Implementation: Toggle blink_state every 500ms
   - Ideal LED: 5% red (constant, no animation)

   **Zone 1 (Too Close):**
   
   - Pattern: Two black (off) LEDs move toward ideal zone
   - Implementation: Calculate two LED positions based on animation_step
   - Background: Orange (constant)
   - Ideal LED: 5% red
   - Motion: Circular animation wrapping at zone boundaries

   **Zone 2 (Ideal):**
   
   - Pattern: Static display, no animation
   - Ideal LED: 100% red (bright stop signal)
   - All other LEDs: Off

   **Zone 3 (Too Far):**
   
   - Pattern: Two green (5% brightness) LEDs move toward ideal zone
   - Implementation: Calculate two LED positions based on animation_step
   - Motion: Continuous movement from current zone toward ideal
   - Ideal LED: 5% green

   **Zone 4 (Out of Range):**
   
   - Pattern: Static display, no animation
   - Last LED: 5% blue
   - Ideal LED: 5% green

   **Animation Update Logic:**

   .. code-block:: c

      void update_animation(state, zone, elapsed_us) {
          if (elapsed_us >= 100000) {  // 100ms elapsed
              state->animation_step++;
              state->last_update_time = esp_timer_get_time();
              
              // Zone 0 blink logic
              if (zone == 0 && (state->animation_step % 5) == 0) {
                  state->blink_state = !state->blink_state;  // Toggle every 500ms
              }
          }
      }

   **Validation:** Animation frame rate consistent at 10 Hz, smooth motion without flicker, 
   blink timing accurate within ±10ms, no performance degradation over extended operation.

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
