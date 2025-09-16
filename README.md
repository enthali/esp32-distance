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

- **Framework**: ESP-IDF v5.4.1
- **Language**: C
- **IDE**: Visual Studio Code with ESP-IDF extension
- **Target**: ESP32
- **Container Support**: Docker Dev Container and GitHub Codespaces ready

## Development Environment

### Dev Container (Recommended)
- **Framework**: ESP-IDF v5.4.1 pre-installed in container
- **Cross-platform**: Works on Windows, Mac, Linux
- **Consistent**: Same environment for all developers
- **No local setup**: No need to install ESP-IDF locally

### USB Serial Connection for Flashing
For Windows users, USB devices need to be attached to the dev container:

#### Option 1: Automated Script (Recommended)
1. **Double-click** `tools/attach-esp32.bat` (easiest)
   - OR **Right-click** on `tools/attach-esp32.ps1` → **"Run as Administrator"**
2. **Follow prompts** - the script will automatically find and attach your ESP32
3. **Done!** Your ESP32 is now available as `/dev/ttyUSB0` in the container

#### Option 2: Manual Setup
1. **Install usbipd-win** (one-time setup):
   ```powershell
   # In Administrator PowerShell
   winget install usbipd
   ```

2. **Connect ESP32** and find the device:
   ```powershell
   # In Administrator PowerShell
   usbipd list
   # Look for: USB-SERIAL CH340 (COM4) - note the BUSID (e.g., 2-1)
   ```

3. **Share and attach** the device:
   ```powershell
   # In Administrator PowerShell (one-time per device)
   usbipd bind --busid 2-1
   
   # After each USB reconnection
   usbipd attach --wsl --busid 2-1
   ```

4. **Verify in container**:
   ```bash
   ls -la /dev/ttyUSB0  # Should show the ESP32 device
   ```

**Note**: After unplugging/reconnecting the ESP32, use Option 1 (run the script) for quickest reconnection.

### For GitHub Codespaces Users

If you're using GitHub Codespaces, you can flash your ESP32 directly from the browser using the ESP-IDF Web extension:

1. **Build the project** in Codespaces:
   ```bash
   idf.py build
   ```

2. **Use ESP-IDF Web extension** for flashing:
   - Open Command Palette (`Ctrl+Shift+P`)
   - Run `ESP-IDF-Web: Flash` command
   - Select your ESP32 device when prompted by the browser
   - Grant USB access permissions

3. **Monitor output** with:
   - `ESP-IDF-Web: Monitor` command

For detailed setup instructions, see [`.devcontainer/README.md`](.devcontainer/README.md#using-esp-idf-web-extension-in-codespaces).

## Project Structure

```text
main/
├── main.c                           # ESP-IDF required entry point
├── config.h                         # Global configuration  
├── CMakeLists.txt                   # Main component definition
└── components/                      # All custom components
    ├── cert_handler/                # SSL certificate management
    ├── config_manager/              # Configuration management
    ├── display_logic/               # LED display control
    ├── distance_sensor/             # HC-SR04 sensor interface
    ├── led_controller/              # WS2812 LED strip control
    ├── startup_tests/               # Hardware validation tests
    └── web_server/                  # HTTP server + WiFi manager
        ├── wifi_manager.c           # WiFi management (merged here)
        ├── wifi_manager.h
        ├── web_server.c
        ├── web_server.h
        └── www/                     # Web assets
```

## Build and Flash

### In Dev Container (Recommended)

1. **Connect ESP32**: Run `tools/attach-esp32.ps1` as Administrator on Windows

2. **Open in Dev Container**: VS Code will automatically detect the `.devcontainer` configuration

3. **Build and flash**:
   ```bash
   # Build the project
   idf.py build
   
   # Flash and monitor
   idf.py -p /dev/ttyUSB0 flash monitor
   
   # Or combined
   idf.py -p /dev/ttyUSB0 build flash monitor
   ```

4. **Monitor only** (if already flashed):
   ```bash
   idf.py -p /dev/ttyUSB0 monitor
   ```

**Exit monitor**: Press `Ctrl+]` to exit the serial monitor.

### Configuration (Optional)

```bash
# Set target (usually auto-detected)
idf.py set-target esp32

# Open configuration menu
idf.py menuconfig
```

## License

This project is open source and available under the MIT License.

## Author

Created as a learning and demo project for embedded ESP32 development with ESP-IDF, utilizing FreeRTOS, OpenFastTrack for requirements and design documentation, and GitHub Copilot's latest features.

