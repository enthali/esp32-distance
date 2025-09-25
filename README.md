# ESP32 Distance Sensor with LED Display

An ESP32-based IoT device that measures distance using an ultrasonic sensor and displays the results on an addressable LED strip. The system includes WiFi connectivity and a web interface for configuration and monitoring.

## Features

- **Real-time Distance Measurement**: HC-SR04 ultrasonic sensor with millimeter precision
- **Visual Display**: WS2812 LED strip shows distance as position and color
- **WiFi Connectivity**: Automatic network connection with captive portal setup
- **Web Interface**: Mobile-responsive configuration and status interface
- **Embedded Optimization**: Integer-only arithmetic for optimal ESP32 performance

## Hardware Requirements

- ESP32 development board (ESP32-WROOM-32 or compatible)
- 40x WS2812 LED strip
- HC-SR04 ultrasonic sensor
- USB cable for power and programming
- Jumper wires for connections
- External 5V power supply (if needed for LED strip)

## Pin Configuration

| Component | Pin | GPIO |
|-----------|-----|------|
| WS2812 LED Strip | Data | GPIO12 |
| HC-SR04 Trigger | Trigger | GPIO14 |
| HC-SR04 Echo | Echo | GPIO13 |
| Power | VCC | 5V/3.3V |
| Ground | GND | GND |

## How It Works

1. **Power On**: ESP32 starts with hardware initialization and LED test sequence
2. **WiFi Setup**:
   - Connects to stored WiFi credentials automatically
   - Creates "ESP32-Distance-Sensor" access point if no credentials stored
   - Captive portal guides through network setup
3. **Distance Measurement**: Continuous HC-SR04 sensor readings every 100ms
4. **Visual Display**: LED strip shows distance as:
   - **Green LED**: Normal range (10cm-50cm mapped to LEDs 0-39)
   - **Red LED**: Out of range (below 10cm or above 50cm)
5. **Web Interface**: Access at device IP for configuration and monitoring

## Technical Specifications

- **LED Strip**: 40 x WS2812 individually addressable LEDs
- **Sensor Range**: 2cm - 400cm (HC-SR04 specification)  
- **Update Rate**: 10Hz real-time visual feedback
- **Communication**: WiFi with smart AP/STA switching and captive portal
- **Web Interface**: HTTP server with mobile-responsive design
- **Power**: USB or external 5V supply
- **Architecture**: Integer-only arithmetic optimized for ESP32 performance

## Development Environment

**Prerequisites**: Just a browser that supports serial connections (Chrome/Edge recommended)

### Quick Start
1. **Clone the repository**
2. **Open in GitHub Codespaces** (reccomended) or use a local VSC devcontainer
3. **Connect ESP32** via USB 
4. **Build and flash**: `idf.py -p /dev/ttyUSB0 build flash monitor`

> **Note**: The first setup may take a few minutes as the container and toolchain are prepared, but after that, you're ready to build and run your code instantly.

> **Local Dev Container Setup**: For Windows users setting up a local dev container, see .devcontainer/README.md for USB device attachment instructions.

### Workshop
#### Workshop: Hands-On with the ESP32 Distance Sensor

This project is ideal for workshops and classroom demonstrations. Please feel free to explore the related documentation in **/docs/Workshop**

> **Tip:** For remote workshops, Codespaces ensures a consistent environment for all participants.



## Build and Flash

```bash
# Build the project
idf.py build

# Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor

# Monitor only (if already flashed)
idf.py -p /dev/ttyUSB0 monitor
```

**Exit monitor**: Press `Ctrl+]` to exit the serial monitor.

## License

This project is open source and available under the MIT License.

## About

ESP32 embedded development demo featuring ESP-IDF, FreeRTOS, OpenFastTrack requirements engineering, and dev container workflows.

