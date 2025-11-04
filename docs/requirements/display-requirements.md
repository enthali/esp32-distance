# Display System Requirements

**Document ID**: REQ-DSP-ESP32-DISTANCE  
**Version**: 2.0  
**Date**: 2025-08-12  
**Author**: ESP32 Distance Project Team  
**Parent Document**: SRS-ESP32-DISTANCE  

## Document Purpose

This document specifies requirements for the Display System, enabling visual representation of distance measurements through LED strip control with clear user experience definitions and implementation guidelines.

## Requirements Traceability

| Requirement ID | Design Reference | Priority |
|----------------|------------------|----------|
| REQ-DSP-OVERVIEW-01 | DSN-DSP-OVERVIEW-01 | Mandatory |
| REQ-DSP-OVERVIEW-02 | DSN-DSP-OVERVIEW-02 | Mandatory |
| REQ-DSP-VISUAL-01 | DSN-DSP-VISUAL-01 | Mandatory |
| REQ-DSP-VISUAL-02 | DSN-DSP-VISUAL-02 | Mandatory |
| REQ-DSP-VISUAL-03 | DSN-DSP-VISUAL-03 | Mandatory |
| REQ-DSP-VISUAL-04 | DSN-DSP-VISUAL-04 | Mandatory |
| REQ-DSP-ANIM-01 | DSN-DSP-ANIM-01 | Mandatory |
| REQ-DSP-ANIM-02 | DSN-DSP-ANIM-02 | Mandatory |
| REQ-DSP-ANIM-03 | DSN-DSP-ANIM-03 | Mandatory |
| REQ-DSP-ANIM-04 | DSN-DSP-ANIM-04 | Mandatory |
| REQ-DSP-ANIM-05 | DSN-DSP-ANIM-05 | Mandatory |
| REQ-DSP-IMPL-01 | DSN-DSP-IMPL-01 | Mandatory |
| REQ-DSP-IMPL-02 | DSN-DSP-IMPL-02 | Mandatory |
| REQ-DSP-IMPL-03 | DSN-DSP-IMPL-03 | Mandatory |

**Dependencies**:

- REQ-DSP-OVERVIEW-01 depends on REQ-LED-1 (LED hardware initialization)
- REQ-DSP-OVERVIEW-02 depends on REQ-CFG-1 (configuration management system)
- REQ-DSP-VISUAL-01 depends on REQ-SNS-11 (processed distance measurements with smoothing)
- REQ-DSP-VISUAL-03 depends on REQ-CFG-2 (boundary parameter configuration)
- REQ-DSP-VISUAL-04 depends on REQ-CFG-2 (boundary parameter configuration)

---

## System Overview Requirements

### REQ-DSP-OVERVIEW-01: Hardware Platform

**Type**: System  
**Priority**: Mandatory  
**Depends**: REQ-LED-1  
**Description**: The display system SHALL utilize WS2812 addressable LED strip hardware through the LED controller component API as an integrated, reactive component. LED count and brightness are configurable via configuration management system.

**Rationale**: Establishes the hardware foundation for the visual display system, enabling flexible LED strip configurations for different deployment scenarios. The system operates reactively, automatically responding to distance measurements rather than external commands. Display system uses LED controller API for all hardware access, maintaining clear separation of concerns.

**Acceptance Criteria**:

- AC-1: Display system uses LED controller component API for all LED hardware access
- AC-2: LED count and brightness are configurable via configuration management system
- AC-3: System validates LED count is within reasonable range (1-100 LEDs)
- AC-4: LED controller component handles all GPIO pin configuration and timing
- AC-5: Display system depends on LED controller initialization completing successfully
- AC-6: System operates continuously, updating display in real-time as measurements arrive

---

### REQ-DSP-OVERVIEW-02: Configuration Integration

**Type**: Interface  
**Priority**: Mandatory  
**Depends**: REQ-CFG-1  
**Description**: The display system SHALL obtain all operational parameters from the configuration management system.

**Rationale**: Ensures centralized configuration management and eliminates hardcoded values, enabling runtime reconfiguration and consistent system behavior. Parameter validation is handled by the configuration manager. Configuration changes trigger system reset, allowing static memory allocation at startup with no runtime reallocation - preventing memory fragmentation and performance degradation in the embedded environment.

**Acceptance Criteria**:

- AC-1: All display parameters are obtained from configuration manager
- AC-2: System reads configuration at initialization

**Configuration Parameters**:

- `led_count`: Number of LEDs in the strip
- `led_brightness`: LED brightness level
- `min_distance_cm`: Minimum distance threshold
- `max_distance_cm`: Maximum distance threshold

---

## User Experience Requirements

### REQ-DSP-VISUAL-01: Core Visualization Concept

**Type**: User Experience  
**Priority**: Mandatory  
**Depends**: REQ-SNS-11  
**Description**: The display system SHALL illuminate a single LED that represents the current measured distance with real-time updates. The display system consumes processed distance measurements from the distance sensor component without additional filtering or smoothing.

**Rationale**: Provides clear, unambiguous visual feedback where users can immediately understand the current distance measurement through LED position. The distance sensor component owns all measurement processing, filtering, and smoothing (REQ-SNS-11); the display system is responsible solely for visual representation of processed measurements, maintaining clear separation of concerns.

**Acceptance Criteria**:

- AC-1: Only one LED is illuminated at any given time (excluding animation layers)
- AC-2: LED position corresponds to processed distance measurement value from sensor component
- AC-3: Display updates immediately when new processed measurements arrive from sensor queue
- AC-4: Display system SHALL NOT apply additional smoothing or filtering to distance measurements
- AC-5: Display system uses distance measurements directly from sensor component's processed measurement queue

---

### REQ-DSP-VISUAL-02: Dual-Layer Display with Position and Animation

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL use a dual-layer architecture combining position indication and directional animation to guide optimal parking. When distance is outside the ideal zone, the system SHALL display a green LED at the measured position (position layer) and a directional animation (animation layer) at reduced brightness guiding toward the ideal zone.

**Rationale**: Dual-layer display provides both current position feedback and directional guidance. The ideal zone is calculated as a centered range (IDEAL_CENTER_PERCENT ± IDEAL_SIZE_PERCENT/2) to provide a clear target area. Green position indicator shows current location, while low-brightness directional animation (2%) guides without overwhelming the position signal.

**Zone Calculation** (configuration-independent):

- `ideal_size = (led_count * 10) / 100` (10% of strip)
- `ideal_center = (led_count * 30) / 100` (30% position)
- `ideal_start = ideal_center - (ideal_size / 2)`
- `ideal_end = ideal_start + ideal_size - 1`

**Acceptance Criteria**:

- AC-1: Minimum configured distance maps exactly to first LED (position 0)
- AC-2: Maximum configured distance maps exactly to last LED (position led_count-1)
- AC-3: LED position calculated using linear interpolation between positions 0 and led_count-1
- AC-4: Zone 1 "too close" (position < ideal_start): Orange LED at position (50% brightness) + red animation (100% brightness) (see REQ-DSP-ANIM-02)
- AC-5: Zone 2 "ideal" (ideal_start ≤ position ≤ ideal_end): All ideal zone LEDs red (see REQ-DSP-ANIM-03)
- AC-6: Zone 3 "too far" (position > ideal_end): Green LED at position + white animation (see REQ-DSP-ANIM-01)
- AC-7: Zone boundaries calculated using integer arithmetic for embedded efficiency
- AC-8: Ideal zone size = 10% of LED count, centered at 30% position

---

### REQ-DSP-VISUAL-03: Emergency Proximity Warning

**Type**: User Experience  
**Priority**: Mandatory  
**Depends**: REQ-CFG-2  
**Description**: The display system SHALL show a blinking red pattern on every 10th LED when the measured distance is below the configured minimum distance threshold, indicating dangerous proximity.

**Rationale**: Blinking pattern on multiple LEDs (every 10th: positions 0, 10, 20, 30, etc.) provides urgent visual warning that is more attention-grabbing than a single static LED. The 1 Hz blink rate (500ms ON, 500ms OFF) is standard for emergency warnings.

**Acceptance Criteria**:

- AC-1: LEDs at positions 0, 10, 20, 30, ... (every 10th) blink red when distance < min_distance_cm
- AC-2: Blink frequency is 1 Hz (500ms ON, 500ms OFF)
- AC-3: Emergency pattern has highest rendering priority (overrides position and animation layers)
- AC-4: Pattern persists until measurement returns to valid range
- AC-5: See REQ-DSP-ANIM-05 for implementation details

---

### REQ-DSP-VISUAL-04: Out of Range Display

**Type**: User Experience  
**Priority**: Mandatory  
**Depends**: REQ-CFG-2  
**Description**: The display system SHALL show only the directional animation layer (no position indicator) when the measured distance is above the configured maximum distance threshold, indicating the measurement is invalid but direction is known.

**Rationale**: When distance exceeds sensor maximum range, the exact position is unreliable, but the direction (too far) is known. Showing only the blue "too far" animation provides directional guidance without falsely indicating a valid position measurement.

**Acceptance Criteria**:

- AC-1: Blue "too far" animation runs when distance > max_distance_cm (see REQ-DSP-ANIM-01)
- AC-2: No green position indicator is shown (measurement invalid)
- AC-3: Animation provides directional cue to move closer
- AC-4: Display persists until measurement returns to valid range

---

## Animation Requirements

### REQ-DSP-ANIM-01: "Too Far" Directional Animation

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL show a running white LED animation from the far end toward the ideal zone when the vehicle is in the "too far" zone, guiding the user to move closer.

**Rationale**: White color at low brightness (2% = ~5/255) provides directional guidance without overwhelming the green position indicator. Animation runs from LED 39 (or led_count-1) toward ideal_end at 100ms per LED step, creating smooth directional motion that intuitively guides the user closer.

**Acceptance Criteria**:

- AC-1: Animation runs when position > ideal_end (too far zone)
- AC-2: White LED color at 2% brightness (~5/255 RGB values)
- AC-3: Animation starts at LED position led_count-1 (far end)
- AC-4: Animation moves toward ideal_end at 100ms per LED step
- AC-5: Animation loops continuously while in too far zone
- AC-6: Animation stops immediately when entering ideal zone or too close zone
- AC-7: Animation layer renders below position layer (green position LED visible on top)

---

### REQ-DSP-ANIM-02: "Too Close" Directional Animation

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL show a running red LED animation from the near end toward the ideal zone when the vehicle is in the "too close" zone, guiding the user to move away.

**Rationale**: Red color at full brightness (100% = 255/255) provides strong directional guidance and warning. Animation runs from LED 0 toward ideal_start at 100ms per LED step, creating smooth directional motion that intuitively guides the user away. Position indicator rendered at 50% brightness to remain visible but subordinate to the warning animation.

**Acceptance Criteria**:

- AC-1: Animation runs when position < ideal_start (too close zone)
- AC-2: Red LED color at 100% brightness (255/255 RGB values)
- AC-3: Animation starts at LED position 0 (near end)
- AC-4: Animation moves toward ideal_start at 100ms per LED step
- AC-5: Animation loops continuously while in too close zone
- AC-6: Animation stops immediately when entering ideal zone or too far zone
- AC-7: Position indicator rendered at 50% brightness to maintain visibility

---

### REQ-DSP-ANIM-03: Ideal Zone STOP Indication

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL illuminate all LEDs in the ideal zone with red color to provide visual target guidance. The ideal zone SHALL be visible at 2% brightness at all times as a background reference, and SHALL illuminate at 100% brightness when the measured position is within the ideal zone, providing clear STOP signal using universal traffic signal color.

**Rationale**: Red is universally recognized as STOP signal. The ideal zone is always visible at low brightness (2%) to show the driver the target parking position regardless of current location. When vehicle reaches ideal zone, all LEDs in that zone (ideal_start through ideal_end) turn solid red at 100% brightness, providing unmistakable "park anywhere in this zone" indication. No position indicator or animation needed - the entire zone is valid. The brightness increase from 2% to 100% provides clear positive feedback when the target is reached.

**Acceptance Criteria**:

- AC-1: All LEDs from ideal_start to ideal_end illuminate red at 2% brightness at all times (background layer)
- AC-2: When ideal_start ≤ position ≤ ideal_end, ideal zone LEDs illuminate at 100% brightness (255, 0, 0)
- AC-3: Background ideal zone (2%) is visible in Zone 1 (too close) and Zone 3 (too far)
- AC-4: No position indicator shown when in ideal zone (entire zone is valid)
- AC-5: No animation layer shown when in ideal zone (destination reached)
- AC-6: Full brightness ideal zone (100%) has priority over animation and position layers
- AC-7: Background ideal zone (2%) renders below animation and position layers

---

### REQ-DSP-ANIM-04: Dual-Layer Rendering Architecture

**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The display system SHALL implement a multi-layer rendering architecture with clear layer priority: Emergency > Ideal Zone (full) > Position > Animation > Ideal Zone (background), ensuring proper visual compositing.

**Rationale**: Layered rendering architecture allows independent animation and position updates with deterministic priority. Emergency warnings must override all other displays, full brightness ideal zone indication overrides position/animation when reached, position indicator must be visible over animation layer for current location feedback, and background ideal zone provides constant visual reference at lowest priority.

**Acceptance Criteria**:

- AC-1: Ideal zone background layer renders first (base layer, 2% red brightness, always visible)
- AC-2: Animation layer renders second (2-100% brightness depending on zone)
- AC-3: Position layer renders third (orange or green LED at measured position, 50-100% brightness)
- AC-4: Ideal zone full brightness overrides position and animation (all red at 100%)
- AC-5: Emergency pattern renders last (highest priority, overrides all layers)
- AC-6: Single led_show() call per frame updates all LEDs atomically
- AC-7: Frame rate of 10 FPS (100ms per frame) for animation steps
- AC-8: No blocking delays in rendering pipeline (use FreeRTOS timer for animation timing)

---

### REQ-DSP-ANIM-05: Emergency Proximity Blinking Pattern

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-DSP-VISUAL-03  
**Description**: The display system SHALL implement a 1 Hz blinking pattern on every 10th LED position when distance is below minimum threshold, creating urgent visual warning. All other display layers SHALL be suppressed during emergency mode to ensure maximum focus on the warning.

**Rationale**: Blinking pattern implementation requires timing state machine with 500ms ON/OFF cycles. Every 10th LED (0, 10, 20, 30, ...) creates distributed warning pattern visible from any viewing angle. Highest priority ensures emergency always visible. Suppressing ideal zone background and other layers during emergency eliminates visual distractions and focuses driver attention exclusively on the danger warning, enabling faster reaction time.

**Acceptance Criteria**:

- AC-1: Blink state toggles every 500ms (1 Hz frequency)
- AC-2: ON state: LEDs at positions 0, 10, 20, 30, ... show red (255, 0, 0)
- AC-3: OFF state: LEDs at positions 0, 10, 20, 30, ... turn off (0, 0, 0)
- AC-4: Blink timing uses FreeRTOS timer or tick count for accuracy
- AC-5: Pattern overrides all other layers (highest priority)
- AC-6: Only affects multiples of 10 positions (0, 10, 20, 30, ...)
- AC-7: Emergency pattern activates immediately when distance < min_distance_cm
- AC-8: Ideal zone background and all other layers are suppressed during emergency mode

---

## Implementation Requirements

### REQ-DSP-IMPL-01: Task-Based Architecture

**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The display system SHALL implement a FreeRTOS task that blocks waiting for new distance measurements and updates the display accordingly.

**Rationale**: Provides responsive, real-time display updates using efficient blocking I/O patterns, ensuring minimal resource usage and immediate response to sensor data.

**Acceptance Criteria**:

- AC-1: Implementation uses FreeRTOS task for display logic
- AC-2: Task blocks waiting for distance measurement notifications
- AC-3: Task has appropriate priority below measurement task but above any other task for real-time response

---

### REQ-DSP-IMPL-02: LED Buffer Management

**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The display system SHALL maintain an internal LED state buffer and update all LEDs simultaneously using WS2812 serial protocol characteristics.

**Rationale**: WS2812 LEDs use serial daisy-chain protocol requiring complete buffer transmission to update display, ensuring consistent visual state across all LEDs.

**Acceptance Criteria**:

- AC-1: Internal buffer maintains complete LED strip state (all LED colors)
- AC-2: Buffer is transmitted to hardware as single operation per WS2812 protocol

---

### REQ-DSP-IMPL-03: Distance-to-LED Calculation

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-DSP-VISUAL-02, REQ-DSP-VISUAL-03, REQ-DSP-VISUAL-04  
**Description**: The display system SHALL implement linear mapping calculation to convert distance measurements to LED positions as specified in the visual requirements.

**Rationale**: Provides the mathematical foundation for translating distance values into discrete LED positions, implementing the visual behavior defined in REQ-DSP-VISUAL-02/03/04.

**Acceptance Criteria**:

- AC-1: Calculation handles full configured distance range per visual requirements
- AC-2: Result is clamped to valid LED index range [0, led_count-1]  
- AC-3: Edge cases (min/max distances) map exactly to first/last LEDs per visual requirements

---

## Requirements Summary

**Total Requirements**: 14  

- **System Overview**: 2 requirements
- **User Experience**: 4 requirements  
- **Animation**: 5 requirements
- **Implementation**: 3 requirements

**Requirement Types**:

- **System**: 1 requirement
- **Interface**: 1 requirement
- **User Experience**: 8 requirements
- **Implementation**: 4 requirements

**Priority Distribution**:

- **Mandatory**: 14 requirements
- **Optional**: 0 requirements
