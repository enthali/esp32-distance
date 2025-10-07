# ESP32 Distance Sensor Project - GitHub Copilot Instructions

You are working on an ESP32-based IoT distance sensor project that measures distance using an HC-SR04 ultrasonic sensor and displays results on a WS2812 LED strip. The project includes WiFi connectivity, a web interface with captive portal, and is being enhanced with HTTPS security.

## Project Context

### Hardware Stack

- **Microcontroller**: ESP32 WROOM-32F (4MB flash)
- **Distance Sensor**: HC-SR04 ultrasonic sensor
- **LED Display**: WS2812 addressable LED strip (40 LEDs)
- **Connectivity**: WiFi (AP and STA modes)
- **Development Framework**: ESP-IDF v5.4.1

### Software Architecture

- **Component-based design**: Modular components for distance sensor, LED controller, WiFi manager
- **Real-time monitoring**: FreeRTOS tasks for continuous sensor monitoring
- **Web interface**: HTTP server with captive portal for configuration
- **Memory optimized**: 4MB flash configuration with Single App Large partition table

### Key Files and Components

- `main/main.c`: Main application logic and task coordination
- `components/distance_sensor/`: HC-SR04 sensor interface and monitoring
- `components/led_controller/`: WS2812 LED strip control
- `main/wifi_manager.c`: WiFi connectivity and captive portal management
- `main/web_server.c`: HTTP server implementation
- `main/dns_server.c`: DNS server for captive portal functionality

### Documentation Structure

- `docs/requirements/`: OpenFastTrack requirements documentation
  - [`README.md`](../docs/requirements/README.md): Requirements documentation overview and navigation guide
- `docs/`: Technical documentation using OpenFastTrack methodology
- `docs/design/*-design.md`: Technical architecture and system design
- `docs/planning/Features-*.md`: Feature tracking and planning documents

## Development Guidelines

### Coding Standards

[ESP32 Coding Standards](./prompt-snippets/esp32-coding-standards.md)

### Build Instructions

[Detailed Build Instructions](./prompt-snippets/build-instructions.md)

### Development Workflow

[Development Process](./prompt-snippets/development.md)

### Commit Message Format

[Commit Message Guidelines](./prompt-snippets/commit-message.md)

## Project-Specific Instructions

### Memory Management

- **Always consider ESP32 memory constraints** when suggesting code changes
- **Use FreeRTOS-specific memory functions** (`heap_caps_malloc`, `heap_caps_free`)
- **Check available heap** before allocating large buffers
- **Prefer stack allocation** for small, temporary variables
- **Use IRAM_ATTR** sparingly and only when necessary

### ESP-IDF Best Practices

- **Use component-based architecture** for new functionality
- **Include proper error handling** with ESP_ERROR_CHECK() and ESP_LOG functions
- **Follow ESP-IDF naming conventions** (esp_, ESP_, CONFIG_)
- **Use appropriate log levels** (ESP_LOGI, ESP_LOGW, ESP_LOGE, ESP_LOGD)
- **Handle WiFi events properly** using event handlers

### Hardware-Specific Considerations

- **GPIO pin assignments**: Document and validate pin usage before modifications
- **Timing constraints**: HC-SR04 requires precise timing (10μs trigger, timeout handling)
- **LED strip timing**: WS2812 requires specific bit timing (T0H, T1H, TL, TH)
- **Power management**: Consider current draw when adding new peripherals

### Security Implementation

- **Current focus**: HTTPS implementation for secure web interface
- **Certificate management**: Automated self-signed certificate generation
- **Memory efficient SSL**: Optimize SSL buffer sizes for ESP32 constraints
- **HTTP to HTTPS redirection**: Maintain user-friendly access patterns

### Requirements Engineering

- **Methodology**: Follow OpenFastTrack (OFT) requirements engineering methodology
- **Requirement IDs**: Use format `REQ-<AREA>-<NUMBER>` (e.g., `REQ-CFG-1`, `REQ-WEB-3`)
- **Traceability**: Maintain bidirectional traceability from requirements → design → implementation → tests
- **Location**: Requirements stored in `docs/requirements/` with detailed `.md` format
- **Current Focus**: Configuration management requirements (`config-requirements.md`)
- **Implementation Links**: Code must reference specific requirement IDs in comments for traceability

## Current Development Phase

### Active Features (Ready for Enhancement)

- ✅ Distance sensor monitoring with component-based architecture
- ✅ LED strip control with smooth color transitions
- ✅ WiFi manager with captive portal
- ✅ HTTP web interface for configuration
- ✅ Memory optimization (4MB flash, 41% free space)

### In Progress (HTTPS Implementation)

- 🔄 Certificate generation and embedding system
- 🔄 HTTPS server migration from HTTP
- 🔄 HTTP to HTTPS redirect server
- 🔄 Security hardening and browser compatibility

### Planned Features

- 📋 Configuration management system
- 📋 Real-time data streaming
- 📋 JSON API endpoints
- 📋 Advanced web interface features

## Important Notes

### When Suggesting Code Changes

1. **Always consider ESP32 memory constraints** and current usage (41% flash free)
2. **Maintain component-based architecture** - don't put everything in main.c
3. **Use proper ESP-IDF error handling** and logging
4. **Test memory impact** of new features using `idf.py size`
5. **Document GPIO pin usage** when adding new hardware interfaces

### When Working with Network Code

1. **Handle WiFi disconnection gracefully** - implement reconnection logic
2. **Use proper event handling** for WiFi and IP events
3. **Consider both AP and STA modes** in networking code
4. **Implement timeouts** for network operations
5. **Test captive portal compatibility** across different devices

### When Implementing HTTPS Features

1. **Optimize SSL buffer sizes** for ESP32 memory constraints
2. **Use self-signed certificates** for local IoT device security
3. **Implement HTTP→HTTPS redirection** for user experience
4. **Test certificate acceptance** across different browsers
5. **Document security configuration** for end users

## Build Environment

Always use the correct build environment setup for Windows:

```powershell
cmd /c "cd /D C:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py build"
```
