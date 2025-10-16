# Display Design

## Design Traceability

| Design ID | Implements Requirement | Priority |
|-----------|------------------------|----------|
| DSN-DSP-OVERVIEW-01 | REQ-DSP-OVERVIEW-01 | Mandatory |
| DSN-DSP-ARCH-01 | REQ-DSP-IMPL-01 | Mandatory |
| DSN-DSP-ARCH-02 | REQ-DSP-OVERVIEW-02 | Mandatory |
| DSN-DSP-ALGO-01 | REQ-DSP-IMPL-03, REQ-DSP-VISUAL-01, REQ-DSP-VISUAL-02, REQ-DSP-VISUAL-03, REQ-DSP-VISUAL-04 | Mandatory |
| DSN-DSP-ALGO-02 | REQ-DSP-IMPL-02 | Mandatory |
| DSN-DSP-ALGO-03 | REQ-SYS-1 | Mandatory |
| DSN-DSP-API-01 | REQ-DSP-IMPL-01 | Mandatory |
| DSN-DSP-ANIM-01 | REQ-DSP-IMPL-04 | Mandatory |
| DSN-DSP-ANIM-02 | REQ-DSP-ANIM-01, REQ-DSP-ANIM-02 | Mandatory |
| DSN-DSP-ANIM-03 | REQ-DSP-ANIM-03, REQ-DSP-ANIM-04 | Mandatory |
| DSN-DSP-ANIM-04 | REQ-DSP-ANIM-05 | Mandatory |

## Target Design Architecture

### DSN-DSP-OVERVIEW-01: WS2812 Hardware Integration Design

Addresses: REQ-DSP-OVERVIEW-01

Design: WS2812 addressable LED strip as primary display hardware with config-driven parameters.

- LED count: Configurable via `config_manager` API (1-100 LEDs, validated)
- Brightness: Configurable via `config_manager` API (0-255, hardware PWM control)
- GPIO pin: Hardware-specific configuration for WS2812 data line
- Initialization: LED hardware initialized before task starts processing measurements
- Real-time operation: Continuous reactive updates as distance measurements arrive

Validation: LED strip responds to configuration changes, hardware initialization successful.

### DSN-DSP-ARCH-01: Task-Based Architecture Design

Addresses: REQ-DSP-IMPL-01

Design: Implement single FreeRTOS task that blocks on `distance_sensor_get_latest()`.

- Task priority set below measurement task to ensure proper data flow hierarchy
- Task runs continuously until system restart
- Core assignment and stack size from centralized configuration
- Blocking wait pattern eliminates polling overhead and provides immediate response

Validation: Task created successfully, priority hierarchy maintained, blocks efficiently.

### DSN-DSP-ARCH-02: Configuration Integration Design

Addresses: REQ-DSP-OVERVIEW-02, REQ-CFG-2

Design: Use `config_manager` API for all distance range parameters.

- Obtain min/max distance values via `config_get_current()` and use `distance_min_mm` / `distance_max_mm` fields (millimeters)
- Cache config values locally at task startup for performance
- Configuration changes handled via system restart (restart-based architecture)
- Configuration validation responsibility belongs to `config_manager`

Validation: All distance parameters obtained from `config_manager` API, no separate config structures.

### DSN-DSP-ALGO-01: Distance-to-Visual Mapping Algorithm (WHAT to display)

Addresses: REQ-DSP-IMPL-03, REQ-DSP-VISUAL-01/02/03/04

Design:

- Normal range (min ≤ distance ≤ max): LED at calculated position with three-zone color scheme
  - Position Formula: `led_index = (distance_mm - min_mm) * (led_count - 1) / (max_mm - min_mm)`
  - Zone 1 (0 ≤ led_index < led_count/4): RED - "too close" zone
  - Zone 2 (led_count/4 ≤ led_index < led_count/2): GREEN - "ideal positioning" zone
  - Zone 3 (led_count/2 ≤ led_index < led_count): ORANGE - "acceptable but not ideal" zone
  - Zone boundaries use integer division for efficiency: `zone1_end = led_count / 4`, `zone2_end = led_count / 2`
- Below minimum (distance < min): Red LED at position 0
- Above maximum (distance > max): Red LED at position `led_count-1`
- Boundary clamping ensures valid LED positions `[0, led_count-1]`
- Single LED illumination enforced by logic

Validation: Min distance → LED 0, max distance → LED `led_count-1`, linear interpolation between,
            zone colors apply correctly based on position, below/above range → correct red LED positions.

### DSN-DSP-ALGO-02: LED Update Pattern Design (HOW to display)

Addresses: REQ-DSP-IMPL-02

Design:

- Step 1: `led_clear_all()` - set all LEDs to off state
- Step 2: `led_set_pixel(position, color)` - set desired LED from ALGO-01 decision
- Step 3: `led_show()` - transmit complete buffer to WS2812 strip

WS2812 serial protocol requires complete buffer transmission; clear-and-set pattern guarantees only one LED illuminated.

Validation: Only one LED illuminated after each update, WS2812 transmission successful.

### DSN-DSP-ALGO-03: Embedded Arithmetic Architecture Design

Addresses: REQ-SYS-1

Design: Pure integer arithmetic for all distance calculations and display operations.

- Distance representation: `uint16_t` millimeters (0-65535mm)
- Position calculations: Multiplication before division for precision preservation
- Boundary checks: Integer comparisons
- Memory efficiency: 2-byte integers vs 4-byte floats
- Execution speed: Integer ALU operations vs FPU operations
- Deterministic timing: No floating-point precision variations

Rationale: Avoid floating-point on resource-constrained microcontrollers unless necessary.

Validation: All arithmetic operations complete within deterministic time bounds.

### DSN-DSP-API-01: Simplified API Design

Addresses: REQ-DSP-IMPL-01

Design: Single entry point for simplified lifecycle management

- `esp_err_t display_logic_start(void)` - primary public function
- Task runs continuously until system restart
- No complex lifecycle management

Validation: Single function call starts display system, no API complexity.

## Animation Design Architecture

### DSN-DSP-ANIM-01: Animation State Machine Architecture

Addresses: REQ-DSP-IMPL-04

Design: State machine with timing control independent of measurement rate

Architecture Components:
- **State Structure**: `display_state_t` maintains current mode, animation position, timing
- **Display Modes**: 5 distinct modes (static, running forward/backward, blink, ideal steady)
- **Timing Control**: FreeRTOS tick-based timing for frame-rate control
- **State Transitions**: Mode changes within 100ms based on LED position zones
- **Rendering Pipeline**: Separate update (state) and render (display) phases

State Machine Flow:
1. Measurement arrives → Calculate LED position → Determine mode from zone
2. Mode change → Initialize animation state (position, target, timing)
3. Animation loop → Update state based on timing → Render to LEDs
4. Continuous → State updates at animation rate, independent of measurements

Zone-to-Mode Mapping:
- LEDs 0-9 (too close) → `DISPLAY_MODE_RUNNING_BACKWARD`
- LEDs 10-13 (ideal) → `DISPLAY_MODE_IDEAL_STEADY`
- LEDs 14-39 (too far) → `DISPLAY_MODE_RUNNING_FORWARD`
- Below minimum distance → `DISPLAY_MODE_BLINK_PATTERN`
- Above maximum distance → `DISPLAY_MODE_STATIC` (green LED 39)

Timing Architecture:
- Task runs at 20Hz check rate (50ms intervals)
- Animation updates occur at mode-specific rates (10fps running, 1Hz blink)
- LED updates only when animation state changes (reduces unnecessary RMT transmissions)

Validation: Mode transitions < 100ms, animations smooth at specified frame rates, independent of measurement timing.

### DSN-DSP-ANIM-02: Running Light Animation Algorithm

Addresses: REQ-DSP-ANIM-01, REQ-DSP-ANIM-02

Design: Single LED "runner" that moves from measurement position toward ideal zone boundary

Algorithm:
```
1. Initialize: current_position = measurement_position
2. Set target based on mode:
   - RUNNING_FORWARD (too far): target = IDEAL_ZONE_END (LED 13)
   - RUNNING_BACKWARD (too close): target = IDEAL_ZONE_START (LED 10)
3. Animation step (every 100ms):
   - If current_position < target: current_position++
   - Else if current_position > target: current_position--
   - Else (reached target): current_position = measurement_position (loop)
4. Render: Clear all LEDs, set current_position to color, show
```

Color Selection:
- Forward animation (too far): GREEN (safe to move forward)
- Backward animation (too close): RED (warning - back up)

Loop Behavior:
- Animation continuously moves toward target
- Upon reaching target, immediately jumps back to measurement position
- Creates continuous "guiding" effect toward ideal zone

Performance:
- Single LED illuminated per frame (minimal RMT transmission)
- Integer position arithmetic (no floating point)
- 10 fps provides smooth motion without excessive CPU usage

Validation: Single LED moves smoothly at 10fps, correct direction toward ideal zone, loops continuously.

### DSN-DSP-ANIM-03: Zone Boundary Definitions

Addresses: REQ-DSP-ANIM-03, REQ-DSP-ANIM-04

Design: Fixed LED positions define parking zones with specific display behaviors

Zone Layout (40 LEDs, 0-39):
```
[0-9]    = TOO CLOSE   → Running backward animation (red) toward LED 10
[10-13]  = IDEAL ZONE  → Steady 4 red LEDs (no animation)
[14-39]  = TOO FAR     → Running forward animation (green) toward LED 13
> max    = OUT OF RANGE → Steady green LED at position 39
< min    = EMERGENCY   → Blinking pattern LEDs 0,10,20,30
```

Ideal Zone Design:
- Positions 10-13 (4 LEDs) provide clear target area
- Steady red illumination indicates "stop here"
- No animation prevents confusion (animation = "keep moving")
- Central to strip for balanced parking guidance

Boundary Detection:
- `led_index >= 10 && led_index <= 13` → Ideal zone
- `led_index < 10` → Too close zone
- `led_index > 13` → Too far zone

Out-of-Range Handling:
- Distance > max_distance: Green LED 39 (safe to enter)
- Distance < min_distance: Emergency blink pattern
- Changed from red to green for "too far" to indicate safety

Validation: Zone boundaries at LEDs 10-13, correct mode selection, ideal zone has 4 steady LEDs.

### DSN-DSP-ANIM-04: Blink Pattern Implementation

Addresses: REQ-DSP-ANIM-05

Design: Simultaneous blinking of every 10th LED for emergency warning

Pattern Specification:
- LED positions: 0, 10, 20, 30 (every 10th LED)
- Frequency: 1 Hz (500ms ON, 500ms OFF)
- Color: Red (danger warning)
- Synchronization: All pattern LEDs blink together

Implementation:
```
1. State: blink_state (boolean) tracks ON/OFF
2. Timer: Toggle blink_state every 500ms
3. Render ON state: 
   - for (i = 0; i < led_count; i += 10): set_pixel(i, RED)
4. Render OFF state:
   - All LEDs off (clear_all)
```

Rationale for Pattern:
- Every 10th LED ensures visibility across entire strip
- 1 Hz blink rate is standard for emergency warnings
- Simultaneous blinking creates strong visual impact
- Red color indicates immediate danger

Trigger Condition:
- Activated when `distance_mm < config.distance_min_mm`
- Indicates vehicle is dangerously close to wall
- Persists until distance increases above minimum

Performance:
- Blink state update every 500ms (low CPU overhead)
- Pattern LEDs calculated dynamically based on led_count
- Works with any strip length (minimum 4 LEDs for full pattern)

Validation: LEDs 0,10,20,30 blink together at 1Hz in red when distance < minimum.
