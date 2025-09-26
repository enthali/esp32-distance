# ESP32 Distance Sensor Project: Architecture Overview

This document provides a high-level architectural overview of the ESP32 Distance Sensor project.

## System Overview

The project is an IoT device based on the ESP32 WROOM-32F microcontroller. It measures distance using an HC-SR04 ultrasonic sensor and visualizes results on a WS2812 LED strip. The device provides WiFi connectivity (AP/STA), a web interface (with captive portal), and is being enhanced with HTTPS security.

## Architecture Diagram

```mermaid
flowchart TD
    subgraph ESP32 MCU
        A[Distance Sensor Component<br>(HC-SR04)]
        B[LED Controller Component<br>(WS2812)]
        C[WiFi Manager<br>(AP/STA, Captive Portal)]
        D[Web Server<br>(HTTP/HTTPS)]
        E[DNS Server<br>(Captive Portal)]
        F[Main Application<br>(FreeRTOS Tasks)]
    end
    A -- Distance Data --> F
    F -- LED Updates --> B
    F -- WiFi Events --> C
    C -- Network Events --> F
    C -- HTTP Requests --> D
    D -- Web UI/API --> User
    E -- DNS Requests --> User
    User((User Devices))
```

## Key Components

- **Distance Sensor Component**: Interfaces with HC-SR04, provides distance readings via FreeRTOS tasks.
- **LED Controller Component**: Drives WS2812 LED strip, visualizes distance data.
- **WiFi Manager**: Manages AP/STA modes, captive portal, and WiFi events.
- **Web Server**: Hosts configuration UI and API (HTTP/HTTPS).
- **DNS Server**: Handles DNS redirection for captive portal.
- **Main Application**: Coordinates tasks, event handling, and inter-component communication.

## Data Flow

1. **Sensor Measurement**: HC-SR04 readings are acquired in a dedicated FreeRTOS task.
2. **LED Visualization**: Distance data is mapped to LED patterns and displayed in real time.
3. **WiFi & Captive Portal**: Device can operate in AP or STA mode; captive portal is provided for configuration.
4. **Web Interface**: Users access configuration and data via a web UI, secured with HTTPS.
5. **DNS Redirection**: DNS server ensures captive portal compatibility for initial device setup.

## Design Principles

- **Component-based architecture**: Each hardware and software function is modularized.
- **Memory optimization**: Designed for 4MB flash, with careful heap/stack usage.
- **Real-time operation**: Uses FreeRTOS tasks for concurrency and responsiveness.
- **Security**: HTTPS support with self-signed certificates and HTTP→HTTPS redirection.
- **Traceability**: Requirements and design are documented and linked in code (OpenFastTrack methodology).

---

_Last updated: 2024-06_