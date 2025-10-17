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
| REQ-DSP-IMPL-01 | DSN-DSP-IMPL-01 | Mandatory |
| REQ-DSP-IMPL-02 | DSN-DSP-IMPL-02 | Mandatory |
| REQ-DSP-IMPL-03 | DSN-DSP-IMPL-03 | Mandatory |
| REQ-DSP-ANIM-01 | DSN-DSP-ANIM-02 | Mandatory |
| REQ-DSP-ANIM-02 | DSN-DSP-ANIM-02 | Mandatory |
| REQ-DSP-ANIM-03 | DSN-DSP-ANIM-03 | Mandatory |
| REQ-DSP-ANIM-04 | DSN-DSP-ANIM-03 | Mandatory |
| REQ-DSP-ANIM-05 | DSN-DSP-ANIM-04 | Mandatory |
| REQ-DSP-IMPL-04 | DSN-DSP-ANIM-01 | Mandatory |

**Dependencies**:

- REQ-DSP-OVERVIEW-02 depends on REQ-CFG-1 (configuration management system)
- REQ-DSP-VISUAL-03 depends on REQ-CFG-2 (boundary parameter configuration)
- REQ-DSP-VISUAL-04 depends on REQ-CFG-2 (boundary parameter configuration)
- REQ-DSP-IMPL-04 depends on REQ-DSP-ANIM-01, REQ-DSP-ANIM-02, REQ-DSP-ANIM-03, REQ-DSP-ANIM-05

---

## System Overview Requirements

### REQ-DSP-OVERVIEW-01: Hardware Platform

**Type**: System  
**Priority**: Mandatory  
**Description**: The display system SHALL utilize WS2812 addressable LED strip hardware with configurable LED count and brightness as an integrated, reactive component.

**Rationale**: Establishes the hardware foundation for the visual display system, enabling flexible LED strip configurations for different deployment scenarios. The system operates reactively, automatically responding to distance measurements rather than external commands.

**Acceptance Criteria**:

- AC-1: System supports WS2812 addressable LED strips
- AC-2: LED count and brightness are configurable via configuration management system
- AC-3: System validates LED count is within reasonable range (1-100 LEDs)
- AC-4: LED strip communication uses appropriate GPIO pin configuration
- AC-5: System initializes LED hardware before processing distance measurements
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
**Description**: The display system SHALL illuminate a single LED that represents the current measured distance with real-time updates.

**Rationale**: Provides clear, unambiguous visual feedback where users can immediately understand the current distance measurement through LED position.

**Acceptance Criteria**:

- AC-1: Only one LED is illuminated at any given time
- AC-2: LED position corresponds to measured distance value
- AC-3: Display updates immediately when new measurements arrive
- AC-4: All other LEDs remain off during operation

---

### REQ-DSP-VISUAL-02: Normal Range Display with Three-Zone Color Scheme

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL illuminate a single LED at a position linearly proportional to the measured distance when the measurement is within the configured range, with color indicating quality zones to guide optimal positioning.

**Rationale**: Provides intuitive distance visualization where LED position directly correlates to distance magnitude. The three-zone color scheme helps users quickly identify the ideal positioning zone: RED indicates "too close" (0-25% of positions), GREEN indicates "ideal positioning" (25-50% of positions), and ORANGE indicates "acceptable but not ideal" (50-100% of positions).

**Acceptance Criteria**:

- AC-1: Minimum configured distance maps exactly to first LED (position 0)
- AC-2: Maximum configured distance maps exactly to last LED (position led_count-1)
- AC-3: LED position is calculated using linear interpolation between positions 0 and led_count-1
- AC-4: Zone 1 (positions 0 to led_count/4-1): RED color indicates "too close"
- AC-5: Zone 2 (positions led_count/4 to led_count/2-1): GREEN color indicates "ideal positioning"
- AC-6: Zone 3 (positions led_count/2 to led_count-1): ORANGE color indicates "acceptable but not ideal"
- AC-7: Zone boundaries calculated using integer arithmetic for embedded efficiency

---

### REQ-DSP-VISUAL-03: Below Minimum Display

**Type**: User Experience  
**Priority**: Mandatory  
**Depends**: REQ-CFG-2  
**Description**: The display system SHALL illuminate the first LED in red when the measured distance is below the configured minimum distance threshold.

**Rationale**: Provides clear visual indication when measurements are below the useful range, using red color and first position to indicate below-minimum condition.

**Acceptance Criteria**:

- AC-1: First LED (position 0) is illuminated red when distance < min_distance_cm
- AC-2: Only the first LED is illuminated (all others remain off)
- AC-3: Display persists until measurement returns to valid range

---

### REQ-DSP-VISUAL-04: Above Maximum Display

**Type**: User Experience  
**Priority**: Mandatory  
**Depends**: REQ-CFG-2  
**Description**: The display system SHALL illuminate the last LED in red when the measured distance is above the configured maximum distance threshold.

**Rationale**: Provides clear visual indication when measurements are above the useful range, using red color and last position to indicate above-maximum condition.

**Acceptance Criteria**:

- AC-1: Last LED (position led_count-1) is illuminated red when distance > max_distance_cm
- AC-2: Only the last LED is illuminated (all others remain off)
- AC-3: Display persists until measurement returns to valid range

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

## Animation Requirements

### REQ-DSP-ANIM-01: Running Light Animation for "Too Far" Zone

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL provide a running light animation for the "too far" zone that guides the driver to move forward into the ideal parking position with consistent animation duration regardless of distance.

**Rationale**: Provides clear directional guidance to the driver when they are too far from the ideal position, using animation to indicate the direction they should move. Consistent animation duration (~1 second) provides predictable UX and prevents jarring speed changes as vehicle approaches ideal zone.

**Acceptance Criteria**:

- AC-1: Animation completes in ~1 second regardless of distance to ideal zone
- AC-1a: Step delay dynamically calculated based on steps_to_target (distance to ideal zone boundary)
- AC-1b: Step delay clamped between 20ms (50fps max) and 200ms (5fps min) for smooth animation
- AC-2: LED color is green (safe to move forward)
- AC-3: Animation direction is toward ideal zone end boundary (dynamically calculated)
- AC-4: Animation loops continuously while vehicle remains in zone
- AC-5: Single LED moves from measurement position toward ideal zone boundary

---

### REQ-DSP-ANIM-02: Running Light Animation for "Too Close" Zone

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL provide a running light animation for the "too close" zone that warns the driver they need to back up away from the wall with consistent animation duration regardless of distance.

**Rationale**: Provides clear warning and directional guidance when the driver is too close to the wall, using red color and animation to indicate urgency and direction. Consistent animation duration (~1 second) provides predictable UX and prevents jarring speed changes as vehicle moves away from wall.

**Acceptance Criteria**:

- AC-1: Animation completes in ~1 second regardless of distance to ideal zone
- AC-1a: Step delay dynamically calculated based on steps_to_target (distance to ideal zone boundary)
- AC-1b: Step delay clamped between 20ms (50fps max) and 200ms (5fps min) for smooth animation
- AC-2: LED color is red (warning - too close)
- AC-3: Animation direction is toward ideal zone start boundary (dynamically calculated)
- AC-4: Animation loops continuously while vehicle remains in zone
- AC-5: Single LED moves from measurement position toward ideal zone boundary

---

### REQ-DSP-ANIM-03: Steady LED Display for Ideal Zone

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL display steady red LEDs in the ideal zone when the vehicle is in the ideal parking position, with no animation or blinking. The ideal zone size and position SHALL be calculated dynamically based on LED strip configuration.

**Rationale**: Provides clear, unambiguous visual confirmation that the parking position is perfect. Steady display (no animation) indicates "stop here" to the driver. Dynamic zone calculation ensures the system adapts to different LED strip lengths (20-100 LEDs) while maintaining consistent user experience.

**Acceptance Criteria**:

- AC-1: LEDs in dynamically calculated ideal zone illuminate in red
- AC-2: No animation or blinking occurs
- AC-3: Display remains steady while vehicle is in ideal zone
- AC-4: All other LEDs remain off
- AC-5: Ideal zone boundaries calculated dynamically based on LED count
- AC-5a: Zone size = 10% of total LEDs (minimum 4 LEDs)
- AC-5b: Zone center positioned at 25% of LED strip length

---

### REQ-DSP-ANIM-04: Green LED for Out-of-Range (Far)

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL illuminate LED 39 in green (not red) when the distance measurement is beyond the maximum configured range or when the sensor reports out-of-range.

**Rationale**: Green color indicates safe distance - vehicle is still outside the garage and it is safe to enter. Changed from red to avoid confusing "too far but safe" with error conditions.

**Acceptance Criteria**:

- AC-1: LED 39 illuminates in green when distance > max_distance_cm
- AC-2: LED 39 illuminates in green for DISTANCE_SENSOR_OUT_OF_RANGE status
- AC-3: Indicates safe distance to enter garage
- AC-4: No animation occurs (steady green)

---

### REQ-DSP-ANIM-05: Blinking Pattern for Emergency (Too Close)

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL display a blinking pattern on every 10th LED (positions 0, 10, 20, 30) when the distance measurement is below the minimum configured range, indicating immediate danger.

**Rationale**: Emergency warning pattern that is highly visible and distinct from all other display modes. Indicates that the vehicle is dangerously close to the wall and immediate action is required.

**Acceptance Criteria**:

- AC-1: LEDs at positions 0, 10, 20, 30 blink simultaneously
- AC-2: Blink frequency is 1 Hz (500ms ON, 500ms OFF)
- AC-3: LED color is red
- AC-4: Pattern continues until distance increases above minimum threshold

---

### REQ-DSP-IMPL-04: Animation Timing and State Machine

**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The display system SHALL implement an animation state machine with timing control that is independent of the distance measurement rate, ensuring smooth animations and responsive state transitions.

**Rationale**: Animations must run smoothly regardless of how fast or slow distance measurements arrive. State machine provides clean separation of concerns between measurement processing and animation rendering.

**Acceptance Criteria**:

- AC-1: State transitions occur within 100ms of mode change
- AC-2: Animation timing is independent of measurement update rate
- AC-3: System supports all 5 display modes (static, running forward, running backward, blink pattern, ideal steady)
- AC-4: Smooth transitions between modes without flicker or visual artifacts

---

## Requirements Summary

**Total Requirements**: 15  

- **System Overview**: 2 requirements
- **User Experience**: 9 requirements  
- **Implementation**: 4 requirements

**Requirement Types**:

- **System**: 1 requirement
- **Interface**: 1 requirement
- **User Experience**: 9 requirements
- **Implementation**: 4 requirements

**Priority Distribution**:

- **Mandatory**: 15 requirements
- **Optional**: 0 requirements
