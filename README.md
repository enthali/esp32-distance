# ESP32 Distance Sensor Project

[![Documentation](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://enthali.github.io/esp32-distance/)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.4.1-orange)](https://github.com/espressif/esp-idf)

An ESP32 firmware that measures distance with an ultrasonic sensor and displays it on a WS2812 LED strip — designed as a garage parking aid and as a learning project for embedded development with GitHub Codespaces and QEMU emulation.

**👉 [Full Documentation on GitHub Pages](https://enthali.github.io/esp32-distance/) 👈**

---

## Contributing

Contributions are welcome. Please read the [Development Guides](https://enthali.github.io/esp32-distance/90_guides/index.html) before submitting a pull request.

## License

MIT License — see [LICENSE](LICENSE) for details.

---

## ✨ Features

- **Real-time Distance Measurement**: HC-SR04 ultrasonic sensor with millimeter precision
- **Visual Display**: WS2812 LED strip shows distance as position and color
- **WiFi Connectivity**: Automatic network connection with captive portal setup
- **Web Interface**: Mobile-responsive configuration and status interface
- **QEMU Emulation**: Full hardware simulation for development without physical devices
- **Professional Documentation**: Sphinx-Needs requirements traceability and design specifications

## 🛠️ Hardware Requirements

- ESP32 development board (ESP32-WROOM-32 or compatible)
- 40x WS2812 LED strip
- HC-SR04 ultrasonic sensor
- USB cable for power and programming
- Jumper wires for connections
- External 5V power supply (recommended for LED strip)

## 📌 Pin Configuration

| Component | Pin | GPIO |
|-----------|-----|------|
| WS2812 LED Strip | Data | GPIO12 |
| HC-SR04 Trigger | Trigger | GPIO14 |
| HC-SR04 Echo | Echo | GPIO13 |
| Power | VCC | 5V/3.3V |
| Ground | GND | GND |

## 🚦 How It Works

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

## 📊 Technical Specifications

- **LED Strip**: 40 x WS2812 individually addressable LEDs
- **Sensor Range**: 2cm - 400cm (HC-SR04 specification)
- **Update Rate**: 10Hz real-time visual feedback
- **Communication**: WiFi with smart AP/STA switching and captive portal
- **Web Interface**: HTTP server with mobile-responsive design
- **Power**: USB or external 5V supply
- **Memory**: Optimized for 4MB flash ESP32 modules
- **Architecture**: Component-based design with FreeRTOS task management

## 🚀 Quick Start

### Prerequisites

Just a browser that supports serial connections (Chrome/Edge recommended) for GitHub Codespaces.

### Using GitHub Codespaces (Recommended)

1. **Fork or clone this repository**
2. **Open in GitHub Codespaces** - Click Code → Codespaces → Create codespace
3. **Wait for setup** (~2-5 minutes first time - ESP-IDF and tools are installed automatically)
4. **Choose your path**:

   **Option A: Test in QEMU (No Hardware Needed)**

   ```bash
   # Build and run in QEMU emulator
   idf.py build
   # Use VS Code task: "Run QEMU (No Debug)"
   ```

   **Option B: Flash Real Hardware**

   ```bash
   # Connect ESP32 via USB, then:
   idf.py -p /dev/ttyUSB0 build flash monitor
   ```

> **First time?** The container setup includes ESP-IDF v5.4.1, QEMU emulator, and all development tools pre-configured.

### 🐛 QEMU Emulation & Debugging

**No Hardware? No Problem!** This project includes full QEMU emulation support:

- **Hardware Simulation** - Distance sensor with animated sweep (5cm→60cm)
- **LED Visualization** - Terminal-based LED display with emoji blocks 🔴🟢🔵
- **Network Stack** - Full TCP/IP via UART tunnel, web interface accessible
- **GDB Debugging** - Set breakpoints and step through code in Codespaces

Access emulated web interface at `http://localhost:8080` when QEMU is running.

See [QEMU Emulator Guide](https://enthali.github.io/esp32-distance/90_guides/qemu-emulator.html) for details.

## 📚 Documentation

**👉 [Full Documentation on GitHub Pages](https://enthali.github.io/esp32-distance/) 👈**

- **[Requirements](https://enthali.github.io/esp32-distance/11_requirements/index.html)** - System and component specifications
- **[Design](https://enthali.github.io/esp32-distance/12_design/index.html)** - Architecture and technical design
- **[API Reference](https://enthali.github.io/esp32-distance/21_api/index.html)** - Component interfaces
- **[Development Guides](https://enthali.github.io/esp32-distance/90_guides/index.html)** - QEMU, debugging, devcontainer setup
- **[Traceability](https://enthali.github.io/esp32-distance/31_traceability/index.html)** - Requirements-to-implementation mapping

## 🏫 Workshop

This project is ideal for workshops and classroom demonstrations with hands-on exercises.

> **Coming Soon:** Workshop materials will be available in the documentation. For now, explore the code and build the project!

**Workshop Benefits:**

- **5-minute setup** with GitHub Codespaces - no local install needed
- **Hardware optional** - QEMU emulation for remote participants
- **Professional practices** - Requirements engineering, component architecture, testing
- **GitHub Copilot integration** - Learn AI-assisted embedded development

## 🔥 Build and Flash

```bash
# Build the project
idf.py build

# Flash to hardware and monitor serial output
idf.py -p /dev/ttyUSB0 flash monitor

# Monitor only (if already flashed)
idf.py -p /dev/ttyUSB0 monitor

# Exit monitor: Press Ctrl+]
```

## 📁 Project Structure

```text
esp32-distance/
├── main/
│   ├── main.c                          # Application entry point
│   └── components/
│       ├── distance_sensor/            # HC-SR04 sensor interface
│       ├── led_controller/             # WS2812 LED strip control
│       ├── display_logic/              # Distance-to-LED mapping
│       ├── config_manager/             # Configuration persistence (NVS)
│       ├── web_server/                 # HTTP server and WiFi manager
│       └── startup_tests/              # Boot-time system checks
├── docs/                               # Sphinx documentation
│   ├── 11_requirements/                # System requirements (Sphinx-Needs)
│   ├── 12_design/                      # Design specifications
│   ├── 21_api/                         # API reference
│   └── 90_guides/                      # Development guides
├── tools/
│   ├── qemu/                           # QEMU emulation scripts
│   └── certificates/                   # Certificate generation
└── .devcontainer/                      # GitHub Codespaces config
```

## 🤖 GitHub Copilot Ready

This project includes comprehensive AI-assisted development instructions in `.github/copilot-instructions.md`. Ask Copilot for help with:

- ESP32-specific patterns and best practices
- Component architecture and memory optimization
- Requirements traceability and documentation
- QEMU emulation and debugging workflows

## 🤝 Contributing

Contributions are welcome! Whether it's bug fixes, new features, or documentation improvements:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes following the coding standards
4. Test in both QEMU and hardware (if available)
5. Update documentation as needed
6. Submit a pull request

See [Development Guides](https://enthali.github.io/esp32-distance/90_guides/index.html) for more details.

## 📄 License

This project is open source and available under the MIT License. See [LICENSE](LICENSE) file for details.

## 🎓 About

ESP32 embedded development demonstration featuring:

- **ESP-IDF v5.4.1** - Official Espressif IoT Development Framework
- **FreeRTOS** - Real-time operating system for task management
- **Sphinx-Needs** - Professional requirements engineering methodology
- **GitHub Codespaces** - Cloud-based development environment
- **QEMU Emulation** - Hardware-optional development and testing
- **GitHub Copilot** - AI-assisted coding and learning

Perfect for learning embedded systems, IoT development, and modern software engineering practices!

---

**Ready to build?** Start with the [Quick Start](#-quick-start) above or explore the [documentation](https://enthali.github.io/esp32-distance/)! 🚀

**Have fun, experiment, and happy coding!** 🎉
