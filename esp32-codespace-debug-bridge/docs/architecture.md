# ESP32 Codespace Debug Bridge Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           ESP32 Codespace Debug Bridge                          │
│                                                                                 │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────────────────┐  │
│  │   Local Machine │    │   GitHub        │    │       ESP32 Hardware        │  │
│  │   (Developer)   │    │   Codespace     │    │                             │  │
│  └─────────────────┘    └─────────────────┘    └─────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

## Detailed Data Flow Architecture

```
ESP32 Target                        Developer Laptop                     GitHub Codespace (Cloud)
┌─────────────┐                   ┌─────────────────────┐                ┌─────────────────────────────────────────┐
│             │                   │                     │                │                                         │
│    App      │                   │  ┌───────────────┐  │         HTTPS  │  ┌───────────────────────────────────┐  │
│  Firmware   │                   │  │ Web Browser   │  │◄───────────────┤  │        Bridge Server              │  │
│             │                   │  │ (Port 8082)   │  │                │  │                                   │  │
│ ┌─────────┐ │                   │  │ ┌───────────┐ │  │                │  │    ┌──────────────────────────┐   │  │
│ │ GDB     │ │                   │  │ │JavaScript │ │  │◄───────────────┤  │    │    Binary Data Bridge    │   │  │
│ │ Stub    │ │                   │  │ │WebSerial  │ │  │     Port 8081  │  │    │ (Transparent Passthrough)│   │  │
│ └─────────┘ │                   │  │ │Bridge     │ │  │                │  │    └──────────┬───────────────┘   │  │
│      ▲      │                   │  │ └───────────┘ │  │                │  │               │                   │  │
│      │      │                   │  │       │       │  │                │  │ ┌─────────────▼─┐ ┌─────────────┐ │  │
│      │      │                   │  │       │       │  │                │  │ │ WebSocket     │ │  TCP Server │ │  │
│      │      │                   │  │ ┌───────────┐ │  │                │  │ │   Server      │ │ (Port 3333) │ │  │
│      │      │                   │  │ │ WebSerial │ │  │                │  │ │ (Port 8081)   │ └─────────────┘ │  │
│      │      │                   │  │ │   API     │ │  │                │  │ └───────────────┘         ▲       │  │
│      │      │                   │  │ └───────────┘ │  │                │  │                           │       │  │
│      │      │                   │  │       │       │  │                │  │                           │       │  │
│      │      │                   │  │       │       │  │                │  │                           │       │  │
│      │      │                   │  │       │       │  │                │  │            ┌─────────────────────┐   │
│      │      │                   │  │       │       │  │                │  │            │        GDB          │   │
│      │      │                   │  │       │       │  │                │  │            │(xtensa-esp32-elf-gdb)│  │
│      │      │                   │  │       │       │  │                │  │            │   (TCP Client)      │   │
│      │      │                   │  │       ▼       │  │                │  │            └─────────────────────┘   │
│      │      │  Serial UART      │  │ ┌───────────┐ │  │                │  │                                      │
│      │      │◄──────────────────┤  │ │USB-Serial │ │  │                │  │                                      │
│      │      │  (115200 baud)    │  │ │ Adapter   │ │  │                │  │                                      │
│      │      │                   │  │ └───────────┘ │  │                │  │                                      │
│      │      │                   │  │               │  │                │  │                                      │
└──────┼──────┘                   │  └───────────────┘  │                └─────────────────────────────────────────┘
       │                          │                     │
       │                          └─────────────────────┘
       │                                    
       └────────────────────────────────────┘
```

## Port Usage Summary

- **Port 8082**: HTTP server serving web client files (HTML, JS, CSS) to browser
- **Port 8081**: WebSocket connection for serial data bridge
- **Port 3333**: TCP connection for GDB remote debugging protocol
- **USB Serial**: Physical connection from laptop to ESP32

## Protocol Stack

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              Protocol Layers                                    │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  Browser                 Codespace                        ESP32                │
│  ┌─────┐                 ┌─────┐                         ┌─────┐               │
│  │     │  Web Serial     │     │                         │     │               │
│  │     │◄────────────────┤     │                         │     │               │
│  │     │  (Raw Bytes)    │     │                         │     │               │
│  │     │                 │     │                         │     │               │
│  │     │  WebSocket      │     │  GDB Remote Protocol    │     │               │
│  │     │◄────────────────┤     │◄────────────────────────┤     │               │
│  │     │  (Binary)       │     │  (ASCII + Binary)       │     │               │
│  │     │                 │     │                         │     │               │
│  │     │                 │     │  TCP                    │     │               │
│  │     │                 │     │◄────────────────────────┤     │               │
│  │     │                 │     │                         │     │               │
│  └─────┘                 └─────┘                         └─────┘               │
│   Web                    Bridge                          Debug                 │
│  Client                  Server                          Stub                  │
│                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────┘
```

## Component Breakdown

### Web Client (Browser)
- **Purpose**: Interface between ESP32 hardware and cloud environment
- **Technologies**: HTML5 Web Serial API, WebSocket
- **Functions**:
  - Request USB serial port access from browser
  - Establish WebSocket connection to Codespace
  - Bidirectional data forwarding (Serial ↔ WebSocket)
  - User interface for connection management

### Bridge Server (Node.js)
- **Purpose**: Protocol bridge between WebSocket and TCP
- **Ports**: 
  - 8081 (WebSocket for serial data bridge)
  - 8082 (HTTP server for web client files)
  - 3333 (TCP for GDB connections)
- **Functions**:
  - WebSocket server for browser connections (port 8081)
  - HTTP server for serving web client files (port 8082)
  - TCP server for GDB connections (port 3333)
  - Binary data passthrough with logging
  - Connection state management

### GDB Integration
- **Tool**: `xtensa-esp32-elf-gdb`
- **Connection**: TCP client to `localhost:3333`
- **Protocol**: GDB Remote Serial Protocol
- **Features**: Breakpoints, stepping, memory inspection, register access

### ESP32 Target
- **Debug Interface**: GDB Stub (enabled in ESP-IDF)
- **Communication**: UART over USB serial
- **Capabilities**: Runtime debugging, panic handling, task inspection