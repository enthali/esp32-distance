Project Overview
================

Welcome to the ESP32 Distance Sensor project! This section provides everything you need to understand what this project does, why it exists, and how to get started.

.. toctree::
   :maxdepth: 2
   :caption: Overview Topics:

   introduction
   hardware
   quick-start

What Is This Project?
----------------------

The ESP32 Distance Sensor is an IoT device that measures distance using an ultrasonic sensor and displays the result on a beautiful LED strip. Think of it as your garage's new best friend—helping you park your car in just the right spot without any "oops" moments!

**Key Components:**

* 🎯 **HC-SR04 Ultrasonic Sensor** - Measures distance with millimeter precision
* 💡 **WS2812 LED Strip** - 40 individually addressable LEDs for visual feedback
* 📡 **ESP32 Microcontroller** - WiFi-enabled brain running FreeRTOS
* 🌐 **Web Interface** - Mobile-responsive configuration and monitoring

Who Is This For?
----------------

* **Garage user** - End user parking their car using the distance display
* **Maker / builder** - Assembles the hardware and configures the device
* **Developer** - Extends or maintains the firmware and documentation

Why Use This Project?
----------------------

**Learn Modern Embedded Development:**

* ESP-IDF v5.4.1 with FreeRTOS real-time operating system
* Component-based architecture following ESP best practices
* Professional requirements engineering with Sphinx-Needs
* QEMU emulation for hardware-optional development

**Zero-Setup Development:**

* GitHub Codespaces with all tools pre-configured
* No local installation required—just a web browser
* Consistent environment for teams and workshops

**Production-Ready Patterns:**

* Configuration management with NVS storage
* WiFi connectivity with captive portal
* HTTP web server for user interface
* Robust error handling and logging

Quick Navigation
----------------

👉 **New to the project?** Start with :doc:`introduction` to understand the motivation and use case.

👉 **Ready to build?** Check the :doc:`hardware` requirements and pin configuration.

👉 **Want to get started fast?** Jump to :doc:`quick-start` for build and flash instructions.

👉 **Looking for workshop materials?** (Coming soon in docs/05_workshop/)

Project Status
--------------

This project is **actively maintained** and suitable for:

* ✅ Educational workshops and demonstrations
* ✅ Learning ESP32 embedded development
* ✅ Reference implementation for ESP-IDF patterns
* ✅ QEMU emulation and hardware testing
* ⚠️ Production use (HTTP only, HTTPS not yet implemented)

**Built on esp32-template:** This project is based on the `esp32-template <https://github.com/enthali/esp32-template>`_ which provides professional requirements engineering, QEMU emulation, and GitHub Codespaces support.
