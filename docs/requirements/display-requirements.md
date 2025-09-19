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

**Dependencies**:
- REQ-DSP-OVERVIEW-02 depends on REQ-CFG-1 (configuration management system)
- REQ-DSP-VISUAL-03 depends on REQ-CFG-2 (boundary parameter configuration)
- REQ-DSP-VISUAL-04 depends on REQ-CFG-2 (boundary parameter configuration)

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

### REQ-DSP-VISUAL-02: Normal Range Display

**Type**: User Experience  
**Priority**: Mandatory  
**Description**: The display system SHALL illuminate a green LED at a position linearly proportional to the measured distance when the measurement is within the configured range.

**Rationale**: Provides intuitive distance visualization where LED position directly correlates to distance magnitude, using green color to indicate in-range measurements.

**Acceptance Criteria**:
- AC-1: LED color is green for measurements within valid range
- AC-2: Minimum configured distance maps exactly to first LED (position 0)
- AC-3: Maximum configured distance maps exactly to last LED (position led_count-1)
- AC-4: LED position is calculated using linear interpolation between positions 0 and led_count-1

---

### REQ-DSP-VISUAL-03: Below Minimum Display

**Type**: User Experience  
**Priority**: Mandatory  
**Depends**: REQ-CFG-2  
**Description**: The display system SHALL flash every 10th LED in red with a 1-second interval when the measured distance is below the configured minimum distance threshold.

**Rationale**: Provides enhanced visual indication when measurements are below the useful range, using red color and a distinctive flashing pattern across multiple LEDs to create a more noticeable below-minimum condition indicator compared to a single static LED.

**Acceptance Criteria**:
- AC-1: Every 10th LED (positions 0, 10, 20, 30, etc.) flashes red when distance < min_distance_cm
- AC-2: Flash interval is 1 second (500ms on, 500ms off cycle)
- AC-3: Only the designated 10th-position LEDs are illuminated during flash-on state (all others remain off)
- AC-4: Flashing pattern persists until measurement returns to valid range
- AC-5: Flash timing is consistent and does not interfere with normal distance measurement updates

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

## Requirements Summary

**Total Requirements**: 9  

- **System Overview**: 2 requirements
- **User Experience**: 4 requirements  
- **Implementation**: 3 requirements

**Requirement Types**:

- **System**: 1 requirement
- **Interface**: 1 requirement
- **User Experience**: 4 requirements
- **Implementation**: 3 requirements

**Priority Distribution**:

- **Mandatory**: 9 requirements
- **Optional**: 0 requirements
