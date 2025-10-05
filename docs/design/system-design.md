# ESP32 Distance Measurement - Technical Architecture

## Overview

This document describes the technical architecture and implementation details for the ESP32 Distance Measurement with LED Strip Display project.

## Design Traceability

| Design ID | Implements Requirement | Priority |
|-----------|------------------------|----------|
| DSN-SYS-1 | REQ-SYS-1 | Mandatory |
| DSN-SYS-2 | REQ-SYS-2 | Mandatory |
| DSN-SYS-3 | REQ-SYS-3 | Mandatory |
| DSN-SYS-4 | REQ-SYS-4 | Mandatory |
| DSN-SYS-5 | REQ-SYS-5 | Mandatory |
| DSN-SYS-6 | REQ-SYS-6 | Mandatory |
| DSN-SYS-7 | REQ-SYS-7 | Mandatory |
| DSN-SYS-8 | REQ-SYS-8 | Mandatory |
| DSN-SIM-LED-01 | REQ-SYS-SIM-1 | Mandatory |
| DSN-SIM-SNS-01 | REQ-SYS-SIM-1 | Mandatory |

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Main Application                      │
├─────────────────────────────────────────────────────────────┤
│  Display Logic Module                                       │
│  - Distance to LED mapping                                  │
│  - Animation control                                        │
│  - Visual feedback patterns                                 │
├─────────────────┬───────────────────┬───────────────────────┤
│  LED Controller │  Distance Sensor  │    Web Server         │
│  Module         │  Module           │    Module             │
│  - WS2812       │  - HC-SR04        │    - HTTP Server      │
│  - RMT Backend  │  - Timing Control │    - WebSocket        │
│  - Color Mgmt   │  - Filtering      │    - JSON API         │
├─────────────────┼───────────────────┼───────────────────────┤
│           ESP-IDF Hardware Abstraction Layer                │
│  RMT | GPIO | Timer | WiFi | HTTP Server | FreeRTOS        │
└─────────────────────────────────────────────────────────────┘
```

## Module Specifications

### 1. LED Controller Module

**Purpose**: Low-level hardware abstraction for WS2812 LED strip control with RAM buffer management.

**Hardware Interface**:
- Configurable GPIO pin for WS2812 data line (RMT backend)
- Configurable number of individually addressable LEDs
- 5V power supply for LED strip

**Key Features**:
- Individual pixel control (set/get/clear)
- RAM buffer maintains current LED state
- Manual update trigger for performance optimization
- Color space management (RGB)
- Configurable LED count during initialization
- Read capability for current LED states

**Current Status**: ✅ **COMPLETED** - Component implemented in `components/led_controller/`

### 2. Distance Sensor Module

**Purpose**: Accurate distance measurement using HC-SR04 ultrasonic sensor with interrupt-driven timing and dual-queue architecture.

**Hardware Interface**:
- GPIO14: Trigger pin
- GPIO13: Echo pin  
- Timing-critical pulse measurement with ESP32 timers

**Key Features**:
- Interrupt-driven measurement with precise timing control
- Dual-queue system for non-blocking API access
- Signal filtering and error detection (timeout, out-of-range, invalid readings)
- Temperature compensation support
- Queue overflow detection and statistics
- Configurable measurement rate (default: 10Hz)
- Background FreeRTOS task (Priority 6) for real-time operation

**Current Status**: ✅ **COMPLETED** - Component implemented in `components/distance_sensor/`

### 3. Display Logic Module

**Purpose**: Business logic for converting distance measurements to visual LED patterns.

**Key Features**:
- Distance-to-LED position mapping algorithms (10cm-50cm → LEDs 0-39)
- Multiple display modes (running LED, range indicator, error states)
- Smooth transitions between states with real-time updates
- Color coding for different distance ranges (green/blue normal, red errors)
- Animation state management with FreeRTOS task (Priority 3)
- Error visual indicators (sensor timeout, out-of-range conditions)

**Current Status**: ✅ **COMPLETED** - Component implemented in `main/display_logic.h/c`

### 4. Web Server Module

**Purpose**: Remote monitoring and configuration interface with WiFi management and HTTPS security.

**Key Features**:

- **Smart WiFi Management**: Auto-connect to stored credentials with AP fallback
- **Captive Portal**: Automatic configuration page with network scanning
- **HTTPS Server**: Secure mobile-responsive web interface for status and settings
- **HTTP Redirect Server**: Lightweight HTTP server redirecting to HTTPS
- **Credential Storage**: Secure WiFi credential management in NVS flash
- **Network Switching**: Seamless AP ↔ STA mode transitions
- **DNS Server**: Captive portal detection and auto-redirect
- **Reset Functionality**: Clear stored credentials and restart system
- **Certificate Management**: Automated self-signed certificate generation and embedding

**Security Implementation**:
- HTTPS server on port 443 with embedded SSL certificates
- HTTP redirect server on port 80 for user convenience
- Self-signed certificates with 25-year validity period
- Build-time certificate generation (no manual management required)
- Certificate embedding using ESP-IDF EMBED_FILES feature

**Current Status**: 🔄 **IN PROGRESS** (Step 4.2) - HTTPS implementation underway

### 5. Certificate Handler Module

**Purpose**: Automated SSL certificate generation and management for HTTPS security.

**Technical Implementation**:
- **Build-time Generation**: Certificates generated automatically during ESP-IDF build if missing
- **Dual Tool Support**: OpenSSL binary (preferred) or Python cryptography library (fallback)
- **Certificate Embedding**: Uses ESP-IDF EMBED_FILES to embed certificates in firmware
- **Long-term Validity**: 25-year certificate validity period for device lifecycle
- **No Manual Management**: Zero configuration required from developers or users

**Certificate Properties**:
- **Common Name**: ESP32-Distance-Sensor
- **Organization**: ESP32 Distance Project
- **Key Size**: RSA 2048-bit
- **Subject Alternative Names**: DNS (esp32-distance-sensor.local), IP (192.168.4.1)
- **Format**: PEM format for maximum compatibility

**Security Features**:
- Private keys never committed to version control (.gitignore exclusions)
- Fresh certificate generation on clean builds
- Self-signed certificates appropriate for local IoT devices
- Standard SSL/TLS encryption for web traffic

**Current Status**: ✅ **COMPLETED** - Component implemented in `components/cert_handler/`

## Data Flow

```text
HC-SR04 Sensor → Distance Sensor Module → Display Logic Module → LED Controller Module → WS2812 Strip
                                     ↓
                              Web Server Module (HTTPS/HTTP) → HTTP Client (WiFi/Captive Portal)
                                     ↑
                              Certificate Handler → Embedded SSL Certificates
```

## Threading Model

```text
Main Task (Priority 1, Core 0)
├── Distance Sensor Task (Priority 6, Core 0)
│   └── Real-time measurements (100ms intervals)
├── Display Logic Task (Priority 3, Core 0) 
│   └── Event-driven LED visualization (blocking sensor API)
└── WiFi/Web Server Tasks (Priority 2, Core 1)
    ├── WiFi Management (connection/AP switching)
    ├── HTTP Server (request handling)
    └── DNS Server (captive portal)

Core 1: WiFi/Network Stack + ESP-IDF System Tasks
```

## Memory Management

- **LED Buffer**: Dynamic allocation based on configured LED count (3 bytes per LED)
- **Sensor History**: Circular buffer for averaging (configurable size)
- **Web Server**: Dynamic allocation for HTTP responses
- **Stack Sizes**: Optimized per task requirements

## Performance Requirements

- **LED Update Rate**: 10Hz minimum for smooth visual feedback
- **Sensor Sampling**: 10Hz for responsive distance tracking
- **Web Interface**: Reasonable latency for monitoring applications
- **Memory Usage**: < 50KB total application memory

## Configuration Management

All configurable parameters exposed through:

- Compile-time defines in header files
- Runtime configuration via web interface
- ESP-IDF menuconfig integration

---

## Implementation Notes

- **LED Controller**: Hardware abstraction documented in `led_controller.h`
- **Distance Sensor**: Timing-critical implementation documented in `distance_sensor.h`  
- **Display Logic**: Business logic patterns documented in `display_logic.h`
- **Web Server**: HTTPS/HTTP API documented in `web_server.h`
- **Certificate Handler**: SSL certificate management documented in `cert_handler.h`

Each module provides comprehensive API documentation in its respective header file.
