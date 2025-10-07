# Configuration Management Requirements

**Document ID**: REQ-CFG-ESP32-DISTANCE  
**Version**: 1.0  
**Date**: 2025-07-24  
**Author**: ESP32 Distance Project Team  
**Parent Document**: SRS-ESP32-DISTANCE  

## Document Purpose

This document specifies detailed requirements for the Configuration Management System, enabling centralized parameter management, runtime configuration, and persistent storage.

## Requirements Traceability

| Requirement ID | Design Reference | Priority |
|----------------|------------------|----------|
| REQ-CFG-1      | DSN-CFG-1        | Mandatory |
| REQ-CFG-2      | DSN-CFG-1        | Mandatory |
| REQ-CFG-3      | DSN-CFG-2, DSN-CFG-3 | Mandatory |
| REQ-CFG-4      | DSN-CFG-3        | Mandatory |
| REQ-CFG-5      | DSN-CFG-4        | Mandatory |
| REQ-CFG-6      | DSN-CFG-4        | Mandatory |
| REQ-CFG-7      | DSN-CFG-5        | Mandatory |
| REQ-CFG-8      | DSN-CFG-5        | Mandatory |
| REQ-CFG-9      | DSN-CFG-5        | Mandatory |
| REQ-CFG-10     | DSN-CFG-6        | Mandatory |
| REQ-CFG-11     | DSN-CFG-6        | Mandatory |

**Dependencies**:

- REQ-CFG-2 depends on REQ-CFG-1 (cannot use centralized configuration that doesn't exist)
- REQ-CFG-3 depends on REQ-CFG-1 (runtime structure must match compile-time constants)
- REQ-CFG-4 depends on REQ-CFG-3 (cannot store configuration structure that doesn't exist)
- REQ-CFG-5 depends on REQ-CFG-4 (cannot implement NVS API without NVS storage requirement)
- REQ-CFG-6 depends on REQ-CFG-3 (cannot validate parameters without defined structure)
- REQ-CFG-7 depends on REQ-CFG-5 (web interface requires configuration API)
- REQ-CFG-8 depends on REQ-CFG-7 (save functionality requires web interface)
- REQ-CFG-9 depends on REQ-CFG-7 (reload/reset functionality requires web interface)

---

**Design references**: Implementation-specific details such as exact storage types, NVS packed layouts, and fixed-point scaling conventions are documented in `docs/design/config-design.md` (DSN-CFG-3). Requirements intentionally remain implementation-agnostic; design document implements the requirements.

## Static Configuration Management

### REQ-CFG-1: Centralized Configuration Header

**Type**: Implementation  
**Priority**: Mandatory  
**Description**: The system SHALL consolidate all user-configurable parameters and system-level configuration values into a single header file `main/config.h`, using integer millimeter architecture for embedded performance optimization, while hardware-specific constants and protocol specifications remain in their respective component files.

**Rationale**: Eliminates scattered magic numbers for configurable parameters throughout the codebase, improving maintainability and reducing configuration errors, while preserving component encapsulation for hardware-specific values.

**Acceptance Criteria**:

- AC-1: Distance sensor parameters (min, max, measurement interval, sensor timeout, ambient temperature, smoothing factor) centralized in device configuration and persistent storage. Implementation-specific names, types, and on-device scaling conventions are defined in `docs/design/config-design.md`.
- AC-2: LED controller parameters (LED count, LED brightness) centralized in device configuration and persistent storage.
- AC-3: WiFi parameters (AP channel, AP max connections, STA retry, STA timeout) centralized in device configuration and persistent storage.
- AC-4: All parameters listed in the Configuration Categories section below SHALL be part of the centralized configuration (logical grouping). Concrete on-device representation and default macro names are specified in `docs/design/config-design.md`.
- AC-5: No additional user-configurable magic numbers remain in source files outside the centralized configuration.
- AC-6: Each configuration value documented with purpose and valid range as shown in Configuration Categories; storage/encoding details are in the design document.

**Configuration Categories**:

The following logical parameter categories are required; concrete macro names, default values, and on-device encoding are specified in `docs/design/config-design.md`.

- Distance sensor configuration: minimum/maximum measurement ranges, measurement interval, sensor timeout, ambient temperature override, smoothing factor
- LED controller configuration: LED count, LED brightness
- WiFi configuration: SSID, password, AP/channel settings, retry and timeout values

**Scope Definition**:

- **Included in config.h**: User-configurable parameters, system behavior settings, timing intervals, network parameters
- **Excluded from config.h**: Hardware timing specifications (WS2812 bit timing), ESP-IDF task stack sizes, protocol constants (HTTP status codes), component-internal buffer sizes

**Note**: This selective approach maintains component encapsulation while centralizing parameters that affect system behavior or user experience.

### REQ-CFG-2: Use of Centralized Configuration

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-CFG-1  
**Description**: All system modules SHALL use centralized configuration values defined in REQ-CFG-1 for parameters specified in the centralized configuration header, while retaining module-specific constants for hardware and protocol specifications.

**Rationale**: Ensures consistent use of configurable parameters across the system while allowing modules to maintain their own module-specific constants.

**Acceptance Criteria**:

- AC-1: All modules SHALL reference config.h for parameters defined therein
- AC-2: Modules SHALL NOT define local copies of centrally-defined configuration values
- AC-3: Modules MAY retain hardware-specific constants (WS2812 timing, task stack sizes, etc.)
- AC-4: Modules MAY retain protocol-specific constants (HTTP status codes, buffer sizes, etc.)
- AC-5: Build process SHALL validate no duplicate definitions of centralized parameters
- AC-6: Each centralized parameter SHALL be used consistently across all referencing modules
- AC-7: Module documentation SHALL clearly distinguish between centralized and local parameters

**Scope Clarification**:

- **Centralized Parameters**: All user-configurable values defined in config.h (distance ranges, LED settings, WiFi parameters)
- **Local Parameters**: Hardware timings, protocol constants, module-internal buffer sizes, task priorities
- **Example**: `DEFAULT_LED_COUNT` must come from config.h, but `WS2812_T0H_NS` remains in led_controller component

---

## Dynamic Configuration Management

### REQ-CFG-3: Configuration Data Structure

**Type**: Design  
**Priority**: Mandatory  
**Depends**: REQ-CFG-1  
**Description**: The system SHALL define a runtime configuration structure containing all user-configurable parameters defined in REQ-CFG-1. The configuration structure SHALL include metadata for versioning and change tracking. Concrete storage layout, data types, alignment, and fixed-point scaling conventions are documented in `docs/design/config-design.md`.

**Rationale**: Enables runtime parameter modification while maintaining consistency with compile-time defaults, using integer millimeter architecture for embedded performance optimization, and ensuring efficient storage in ESP32 NVS flash memory.

**Acceptance Criteria**:

- AC-1: Configuration structure includes all runtime-modifiable parameters from REQ-CFG-1
- AC-2: Structure includes metadata for versioning and change tracking
- AC-3: Data representation chosen for NVS storage efficiency and embedded performance; concrete type selections and packing are specified in `docs/design/config-design.md`
- AC-4: String fields sized appropriately for WiFi credentials and documented in the design doc
- AC-5: Default values match compile-time defaults specified in the design document
- AC-6: Structure layout compatible with ESP32 memory alignment requirements

**Configuration Structure (conceptual)**:

The runtime configuration SHALL include metadata and the following parameter groups:

- Configuration metadata: version and save count for migration and tracking
- Distance sensor settings: minimum/maximum measurement ranges, measurement interval, sensor timeout, ambient temperature, smoothing factor
- LED settings: LED count and brightness
- WiFi settings: SSID, password, AP/channel settings, retry and timeout values

Specific storage types, packed layouts, and fixed-point scaling conventions are design decisions and are documented in the configuration design specification: `docs/design/config-design.md` (see DSN-CFG-3).

### REQ-CFG-4: Non-Volatile Storage (NVS)

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-CFG-3  
**Description**: The system SHALL store runtime configuration defined in REQ-CFG-3 in ESP32 NVS flash memory with persistence across power cycles, and SHALL load configuration during system startup.

**Rationale**: Maintains user configuration permanently without requiring firmware recompilation, using the configuration structure defined in REQ-CFG-3. Ensures configuration is available to all components during startup sequence.

**Acceptance Criteria**:

- AC-1: Configuration saved to NVS namespace "esp32_distance_config"
- AC-2: Configuration survives device reset and power loss
- AC-3: NVS write operations are atomic and protected against power loss
- AC-4: Configuration integrity maintained through power cycles
- AC-5: System loads configuration from NVS during startup sequence
- AC-6: Configuration available to all components after successful startup load
- AC-7: Automatic fallback to defaults if NVS read fails (including first-time startup)

### REQ-CFG-5: Configuration API

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-CFG-4  
**Description**: The system SHALL provide a well-defined API for configuration management operations with robust error handling and automatic fallback to defaults.

**Rationale**: Enables consistent configuration access across all system components while handling NVS storage errors gracefully.

**Acceptance Criteria**:

- AC-1: config_init() initializes configuration subsystem and loads configuration from NVS
- AC-2: config_load() reads configuration from NVS with error handling
- AC-3: config_load() SHALL call config_factory_reset() when NVS read fails or validation fails
- AC-4: config_save() writes configuration to NVS with validation by calling config_validate_range() before save
- AC-5: config_validate_range() validates all parameter ranges
- AC-6: config_factory_reset() restores compile-time defaults from REQ-CFG-1 and persists them to NVS via config_save()
- AC-7: All API functions return appropriate esp_err_t codes
- AC-8: Thread-safe access with mutex protection
- AC-9: Error recovery sequence (load failure → factory reset → save defaults) completes atomically

**API Specification**:

```c
// Configuration management API
esp_err_t config_init(void);
esp_err_t config_load(system_config_t* config);
esp_err_t config_save(const system_config_t* config);
esp_err_t config_validate_range(const system_config_t* config);
esp_err_t config_factory_reset(void);
bool config_is_valid_range(const char* param_name, int32_t value, int32_t min_val, int32_t max_val);
```

### REQ-CFG-6: Parameter Validation

**Type**: Implementation  
**Priority**: Mandatory  
**Depends**: REQ-CFG-3  
**Description**: The system SHALL validate all configuration parameters in the runtime configuration structure against defined ranges before acceptance.

**Rationale**: Prevents invalid configurations that could cause system malfunction or instability by enforcing parameter bounds and inter-parameter relationships.

**Acceptance Criteria**:

- AC-1: All integer parameters validated against min/max ranges as specified below
- AC-2: All fixed-point parameters validated against min/max ranges as specified below
- AC-3: Inter-parameter validation (e.g., distance_min_mm < distance_max_mm, sensor_timeout_ms < measurement_interval_ms)
- AC-4: Invalid parameters rejected with specific error messages
- AC-5: Validation performed before NVS save operations
- AC-6: Validation errors logged with parameter name and attempted value

**Parameter Validation Ranges**:

The system SHALL validate parameters against the following human-oriented ranges. The design document `docs/design/config-design.md` specifies exact on-device representations and any fixed-point scaling used for storage.

- distance_min_mm: 50 - 1000 (millimeters)
- distance_max_mm: 200 - 4000 (millimeters) (must be > distance_min_mm)
- measurement_interval_ms: 50 - 1000 (milliseconds)
- sensor_timeout_ms: 10 - 50 (milliseconds) (must be < measurement_interval_ms)
- temperature_c: -20.0 - 60.0 (degrees Celsius) ; storage encoding documented in design doc
- smoothing_factor: 0.001 - 1.0 (normalized smoothing parameter) ; storage scaling documented in design doc

- led_count: 1 - 60
- led_brightness: 10 - 255

- wifi_ap_channel: 1 - 13
- wifi_ap_max_conn: 1 - 10
- wifi_sta_max_retry: 1 - 10
- wifi_sta_timeout_ms: 1000 - 30000 (milliseconds)

---

## Configuration User Interface

### REQ-CFG-7: Web Settings Page

**Type**: Implementation  
**Priority**: High  
**Description**: The system SHALL provide a web interface for runtime configuration modification.

**Rationale**: Enables user-friendly configuration without requiring technical knowledge or firmware recompilation.

**Acceptance Criteria**:

- AC-1: Web page accessible at `/settings` endpoint
- AC-2: Form fields for all runtime-configurable parameters
- AC-3: Current values pre-populated in form fields
- AC-4: Parameter ranges displayed as input constraints
- AC-5: Client-side validation before form submission
- AC-6: Server-side validation with error feedback
- AC-7: Success confirmation after configuration save

### REQ-CFG-8: Save Configuration

**Type**: Implementation  
**Priority**: High  
**Description**: The system SHALL save user configuration changes permanently and restart the device to apply them.

**Rationale**: Provides reliable configuration persistence with automatic device restart to ensure all components use the new configuration values.

**Acceptance Criteria**:

- AC-1: Form validation performed before save operation
- AC-2: Configuration written to NVS using config_save() API
- AC-3: Success notification displayed to user
- AC-4: Automatic device restart triggered 3 seconds after save
- AC-5: User feedback during restart countdown
- AC-6: Error handling with specific user feedback for validation failures
- AC-7: All form controls disabled during restart sequence

### REQ-CFG-9: Reload Configuration and Reset to Defaults

**Type**: Implementation  
**Priority**: High  
**Description**: The system SHALL provide reload functionality to refresh the UI with current NVS values and reset functionality to restore factory defaults.

**Rationale**: Enables users to discard unsaved changes by reloading current configuration and provides recovery mechanism through factory reset.

**Acceptance Criteria**:

**Reload Configuration**:

- AC-1: "Reload" button reads current values from NVS
- AC-2: All form fields populated with current NVS values
- AC-3: Any unsaved changes in form are discarded
- AC-4: Success notification confirms reload operation
- AC-5: Error handling for NVS read failures

**Reset to Factory Defaults**:

- AC-6: "Reset to Defaults" button with user confirmation dialog
- AC-7: Confirmation message clearly warns of irreversible action
- AC-8: Factory defaults restored using config_factory_reset() API
- AC-9: Form fields immediately updated with default values
- AC-10: Success notification confirms reset operation
- AC-11: Error handling with user feedback for reset failures

---

## Quality Requirements

### REQ-CFG-10: Configuration Performance

**Type**: Performance  
**Priority**: Mandatory  
**Description**: Configuration operations SHALL not impact real-time system performance.

**Acceptance Criteria**:

- AC-1: Configuration load during startup completes within 100ms
- AC-2: Configuration save operations complete within 500ms
- AC-3: Configuration changes applied without sensor measurement interruption
- AC-4: Web configuration interface remains responsive during operations

### REQ-CFG-11: Configuration Reliability

**Type**: Reliability  
**Priority**: Mandatory  
**Description**: Configuration system SHALL handle all error conditions gracefully.

**Acceptance Criteria**:

- AC-1: NVS corruption detected and reported with recovery action
- AC-2: Invalid parameter ranges rejected with specific error messages
- AC-3: Power loss during configuration save does not corrupt NVS
- AC-4: Network disconnection during web configuration handled gracefully
- AC-5: System operates with default configuration if NVS unavailable

---

*This document follows OpenFastTrack methodology for requirements engineering and traceability.*
