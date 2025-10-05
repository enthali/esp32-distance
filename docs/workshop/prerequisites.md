
# Workshop Prerequisites

## 🖥️ SW and Licenses


- **A Supported Browser**

    To use browser-based serial port access and the ESP-IDF Web extension, you must use a browser that supports the Web Serial API. **Supported browsers** include the latest versions of **Google Chrome**, **Microsoft Edge**, **Opera**, and **Arc Browser**.
    
    Firefox, Safari, and most mobile browsers are **not supported** for direct device communication. For the best experience, use Chrome or Edge on desktop.

- **GitHub Account**

    You will need a GitHub account to participate in the workshop. If you do not already have one, you can sign up for free at [github.com](https://github.com/). The free GitHub account is sufficient for all workshop activities, including accessing the repository, using Codespaces (within the free usage quota), and collaborating with others. 

- **GitHub Copilot Subscription**

    GitHub Copilot is an AI-powered coding assistant that helps you write code faster and with fewer errors by providing real-time code suggestions, explanations, and even generating entire functions or files. It is integrated into VS Code and Codespaces, making it easy to use during the workshop.

    For most workshop tasks, the free tier of Copilot is sufficient you can use Chat, Edit and Agent mode in VSCode. However, to access advanced features such as the Copilot coding agent, you will need at least a GitHub Copilot Pro subscription. The Pro subscription is available as a 30-day free trial, allowing you to fully explore Copilot's capabilities during the workshop without any cost.

- **GitHub Codespaces**

    GitHub Codespaces provides a cloud-based development environment pre-configured for this project. It allows you to start coding instantly in your browser or VS Code, with all dependencies (ESP-IDF, toolchain, libraries) already set up. No local installation is required. 


**Everything else is provided inside the preconfigured Codespaces working environment.**

## 🛠️ Hardware

- **ESP32 WROOM-32F Development Board**

    The workshop is designed for the ESP32 WROOM-32F (4MB flash) module. This board provides WiFi/Bluetooth connectivity, multiple GPIOs, and is compatible with the HC-SR04 ultrasonic sensor and WS2812 LED strip. If you have a different ESP32 variant, most exercises will still work, but pin assignments and memory constraints may vary. No hardware is required for simulation or code review tasks, but hands-on flashing and sensor/LED testing require a physical ESP32 board.

- **HC-SR04 Ultrasonic Distance Sensor**

    Used for measuring distance. Connects to ESP32 GPIOs (requires 5V power, but logic-level shifting is handled on most dev boards).

- **WS2812 Addressable LED Strip (40 LEDs recommended)**

    Used for visualizing distance measurements. Connects to a single ESP32 GPIO (data line).

- **USB serial adapter** for flashing 

    Required for uploading firmware and running code on real hardware. Most ESP32 dev boards include a built-in USB-to-serial adapter, but if your board does not, you will also need a **USB serial adapter** (e.g., FTDI, CP2102, CH340) to flash, debug, and monitor the device via serial connection.
