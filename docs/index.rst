ESP32 Distance Sensor
=====================

Welcome to the ESP32 Distance Sensor project! 

This IoT device measures distance using an ultrasonic sensor and displays results on a beautiful LED strip. Perfect for garage parking assistance, learning embedded systems, or hands-on workshops.

🎯 What Is This Project?
-------------------------

An ESP32-powered distance sensor that helps you park your car perfectly every time! It combines:

* **HC-SR04 Ultrasonic Sensor** - Measures distance with millimeter precision
* **WS2812 LED Strip** - 40 individually addressable LEDs for visual feedback
* **ESP32 Microcontroller** - WiFi-enabled brain running FreeRTOS
* **Web Interface** - Mobile-responsive configuration and monitoring

**The Story Behind It:** Born from frustration after one too many bumper-meets-wall incidents in the garage. Sometimes the best projects solve real problems! 🚗

🚀 Quick Start
--------------

**New to the project?** Start here:

1. 📖 **Understand the Project** - Read the :doc:`01_overview/introduction` for context and motivation
2. 🛠️ **Check Hardware Needs** - See :doc:`01_overview/hardware` for components and pin configuration  
3. ⚡ **Build and Run** - Follow :doc:`01_overview/quick-start` to get started in minutes

**No hardware?** Try QEMU emulation! Full simulator with network support included.

✨ Key Features
---------------

**Hardware & Sensors:**

* Real-time distance measurement (HC-SR04, 10Hz update rate)
* Visual LED display (40× WS2812, position + color feedback)
* Configurable range (default: 10cm-50cm mapped to LED strip)

**Connectivity & Interface:**

* WiFi connectivity (AP and STA modes)
* Captive portal for easy setup
* HTTP web interface (mobile-responsive)
* Configuration persistence (NVS storage)

**Development & Testing:**

* **GitHub Codespaces** - Zero-setup cloud development
* **DevContainer** - Alternative setup for your local enviroment (requires Docker)
* **QEMU Emulation** - Hardware-optional development with full network stack
* **GDB Debugging** - Breakpoint debugging in emulator
* **Professional Documentation** - Sphinx-Needs requirements engineering

**Software Architecture:**

* Component-based design (modular, maintainable)
* FreeRTOS task management (real-time operation)
* ESP-IDF v5.4.1 (official framework)
* Optimized for embedded devices (4MB flash configuration, fix point integer...)

📚 Documentation Navigation
----------------------------

**Getting Started:**

* :doc:`01_overview/introduction` - Project story and motivation  
* :doc:`01_overview/hardware` - Hardware specs, BOM, wiring diagrams
* :doc:`01_overview/quick-start` - Build, flash, and run (hardware or QEMU)

**Requirements & Design:**

* :doc:`11_requirements/index` - Formal system requirements (Sphinx-Needs)
* :doc:`12_design/index` - Technical architecture and design specifications
* :doc:`31_traceability/index` - Requirements-to-implementation traceability

**Development:**

* :doc:`90_guides/index` - QEMU emulation, debugging, devcontainer setup
* :doc:`21_api/index` - Component API reference

**Workshop:**

* :doc:`05_workshop/index` - Workshop overview and structure
* :doc:`05_workshop/prerequisites` - Browser, account, and hardware requirements  
* :doc:`05_workshop/getting-started` - Fork, Codespace setup, build, and flash
* :doc:`05_workshop/hardware-setup` - Wiring diagrams and assembly instructions
* :doc:`05_workshop/exercises` - Hands-on challenges with GitHub Coding Agent

🎓 Who Is This For?
-------------------

* **Makers & Tinkerers** - Build a practical IoT device with real-world applications
* **Students & Learners** - Hands-on introduction to embedded systems development
* **Workshop Participants** - Guided learning with professional engineering practices
* **Embedded Developers** - Reference implementation of ESP-IDF best practices

💡 What You'll Learn
---------------------

* **Embedded Systems** - FreeRTOS, hardware interfaces, memory management
* **IoT Connectivity** - WiFi, HTTP servers, network protocols
* **Software Engineering** - Component architecture, requirements engineering, testing
* **Modern Tools** - GitHub Codespaces, QEMU, GDB debugging
* **Vibe-coding embedded** - AI-assisted coding with GitHub Copilot, Coding Agent and using prompt templates

🛠️ Built With
---------------

* **ESP-IDF v5.4.1** - Official Espressif IoT Development Framework
* **FreeRTOS** - Real-time operating system
* **Sphinx-Needs** - Professional requirements engineering
* **QEMU** - Full ESP32 hardware emulation
* **GitHub Codespaces** - Cloud-based development environment

**Based on:** `esp32-template <https://github.com/enthali/esp32-template>`_ - Professional ESP32 project template with lots of features! - check it out

All Requirements Overview
==========================

.. needlist::
   :show_status:
   :show_tags:

.. toctree::
   :maxdepth: 2
   :hidden:

   01_overview/index
   05_workshop/index
   11_requirements/index
   12_design/index
   21_api/index
   31_traceability/index
   90_guides/index
   legal/impressum
   legal/datenschutz
