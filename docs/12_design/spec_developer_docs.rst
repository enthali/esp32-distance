Supporting Documentation Design Specifications
===============================================

This document defines the required content for each traceable supporting
documentation file. One SPEC element per chapter — specifying what each
chapter must cover, not how it is written.

**Document Version**: 1.0
**Last Updated**: 2026-03-16

README.md
---------

.. spec:: README Introduction and Project Link
   :id: SPEC_DEV_README_INTRO
   :links: REQ_DEV_README_1, SPEC_ARCH_LAYERS_1
   :status: approved
   :tags: developer, documentation, readme

   **Description:** The README introduction section SHALL contain a short
   project description (≤3 sentences) and a prominent link to the GitHub
   Pages documentation site.

   **Required content:**

   - Project name and one-line purpose statement
   - What the device does (distance → LED feedback)
   - Link to full documentation on GitHub Pages

   **Explicitly excluded:** Hardware specs, pin tables, build instructions,
   QEMU steps — these live in the Sphinx docs to avoid duplication.

   **Traceability note:** If ``SPEC_ARCH_LAYERS_1`` changes the component
   structure, the project description in README SHALL be reviewed.


.. spec:: README Metadata Section
   :id: SPEC_DEV_README_META
   :links: REQ_DEV_README_1
   :status: approved
   :tags: developer, documentation, readme

   **Description:** The README SHALL end with a metadata section covering
   project governance information.

   **Required content:**

   - License statement with SPDX identifier
   - Contributing pointer (link to CONTRIBUTING.md or inline note)


docs/01_overview/index.rst
---------------------------

.. spec:: Overview Index — Project Description
   :id: SPEC_DEV_OV_INDEX_WHAT
   :links: REQ_DEV_OV_INDEX_1, US_DISPLAY_1
   :status: approved
   :tags: developer, documentation, overview

   **Description:** The "What Is This Project?" section SHALL describe the
   device purpose in plain language for a first-time reader.

   **Required content:**

   - Core function: measures distance, shows result on LED strip
   - Primary use case: parking aid
   - Brief mention of key components (ESP32, ultrasonic sensor, LED strip)


.. spec:: Overview Index — Audience Section
   :id: SPEC_DEV_OV_INDEX_WHO
   :links: REQ_DEV_OV_INDEX_1, US_DEV_1, US_DEV_2
   :status: approved
   :tags: developer, documentation, overview

   **Description:** The "Who Is This For?" section SHALL enumerate target
   audiences so readers can self-select relevant documentation.

   **Required content:**

   - Garage user (end user)
   - Maker / builder (hardware assembler)
   - Developer (firmware / documentation contributor)


.. spec:: Overview Index — Navigation Section
   :id: SPEC_DEV_OV_INDEX_NAV
   :links: REQ_DEV_OV_INDEX_1
   :status: approved
   :tags: developer, documentation, overview

   **Description:** The navigation / quick-links section SHALL provide a
   structured map of the documentation to reduce discovery friction.

   **Required content:**

   - Links to key sections (overview, requirements, design, guides)
   - Brief description of what each section contains


docs/01_overview/hardware.rst
------------------------------

.. spec:: Hardware Components Listing
   :id: SPEC_DEV_OV_HW_COMPONENTS
   :links: REQ_DEV_OV_HW_1, REQ_SNS_1, REQ_LED_1
   :status: approved
   :tags: developer, documentation, hardware

   **Description:** The hardware components section SHALL list all required
   and optional components with specifications.

   **Required content:**

   - ESP32 development board (type, flash size)
   - HC-SR04 ultrasonic sensor (operating voltage, range)
   - WS2812 LED strip (addressable, power requirements)
   - Power supply notes

   **Traceability note:** SHALL be reviewed when ``REQ_SNS_1`` or ``REQ_LED_1``
   change component specifications.


.. spec:: Hardware Pin Assignment Table
   :id: SPEC_DEV_OV_HW_PINS
   :links: REQ_DEV_OV_HW_1, REQ_SNS_1, REQ_LED_1
   :status: approved
   :tags: developer, documentation, hardware

   **Description:** The pin configuration section SHALL provide a complete
   GPIO assignment table for the project.

   **Required content:**

   - GPIO assignment table (pin number, signal name, connected component)
   - Note on ESP32 strapping pins to avoid
   - Wiring diagram reference or inline wiring notes


docs/01_overview/quick-start.rst
---------------------------------

.. spec:: Quick Start — QEMU Path
   :id: SPEC_DEV_OV_QS_QEMU
   :links: REQ_DEV_OV_QS_1, REQ_SYS_SIM_1
   :status: approved
   :tags: developer, documentation, quickstart, qemu

   **Description:** The QEMU emulation path section SHALL provide complete
   steps for a first-time run without hardware.

   **Required content:**

   - Clone / open in Codespaces step
   - Build command
   - QEMU start command or VS Code task
   - How to access the web interface from the host browser

   **Traceability note:** SHALL be reviewed when ``REQ_SYS_SIM_1`` changes
   the emulation setup.


.. spec:: Quick Start — Hardware Path
   :id: SPEC_DEV_OV_QS_HW
   :links: REQ_DEV_OV_QS_1, REQ_SYS_HW_1
   :status: approved
   :tags: developer, documentation, quickstart, hardware

   **Description:** The hardware path section SHALL provide complete steps
   for first-time flashing to physical hardware.

   **Required content:**

   - Build command
   - Flash command (``idf.py flash monitor`` or web flasher reference)
   - Initial WiFi AP setup step


docs/90_guides/qemu-emulator.rst
---------------------------------

.. spec:: QEMU Guide — Starting QEMU
   :id: SPEC_DEV_GUIDE_QEMU_START
   :links: REQ_DEV_GUIDE_QEMU_1, REQ_SYS_SIM_1, SPEC_ARCH_QEMU_1
   :status: approved
   :tags: developer, documentation, qemu

   **Description:** The QEMU start section SHALL document all methods for
   launching the emulator.

   **Required content:**

   - Shell script invocation (``run_qemu.sh`` / ``run_qemu_debug.sh``)
   - VS Code task alternative
   - What the scripts do automatically (network bridge, etc.)


.. spec:: QEMU Guide — Accessing the Web Interface
   :id: SPEC_DEV_GUIDE_QEMU_WEB
   :links: REQ_DEV_GUIDE_QEMU_1, REQ_SYS_SIM_2, REQ_WEB_1
   :status: approved
   :tags: developer, documentation, qemu

   **Description:** The web interface access section SHALL explain how to
   reach the running firmware's HTTP server from the host browser.

   **Required content:**

   - Host URL and port
   - HTTP proxy requirement note (QEMU network architecture)
   - Troubleshooting pointer if connection fails


.. spec:: QEMU Guide — Stopping QEMU
   :id: SPEC_DEV_GUIDE_QEMU_STOP
   :links: REQ_DEV_GUIDE_QEMU_1, REQ_SYS_SIM_1
   :status: approved
   :tags: developer, documentation, qemu

   **Description:** The QEMU stop section SHALL document how to cleanly
   terminate the emulator.

   **Required content:**

   - Shell script invocation (``stop_qemu.sh``)
   - VS Code task alternative


docs/90_guides/switching-dev-modes.rst
---------------------------------------

.. spec:: Dev Modes Guide — How to Switch
   :id: SPEC_DEV_GUIDE_MODES_HOW
   :links: REQ_DEV_GUIDE_MODES_1, REQ_SYS_SIM_1
   :status: approved
   :tags: developer, documentation, devmodes

   **Description:** The switching procedure section SHALL document all
   supported methods for changing between QEMU and hardware build targets.

   **Required content:**

   - ``idf.py menuconfig`` method (recommended)
   - Direct ``sdkconfig`` edit method
   - One-liner command-line method
   - Rebuild requirement after switching


.. spec:: Dev Modes Guide — What Changes Between Modes
   :id: SPEC_DEV_GUIDE_MODES_DIFF
   :links: REQ_DEV_GUIDE_MODES_1, REQ_SYS_SIM_1, REQ_NETIF_TUNNEL_5
   :status: approved
   :tags: developer, documentation, devmodes

   **Description:** The behavioural differences section SHALL explain what
   the build system does differently in each mode.

   **Required content:**

   - Which components are swapped (simulator vs. real driver)
   - Network stack difference (UART tunnel vs. WiFi)
   - ``CONFIG_IDF_TARGET_ESP32_QEMU`` / ``CONFIG_TARGET_EMULATOR`` guard explanation


docs/90_guides/debugging.rst
------------------------------

.. spec:: Debugging Guide — Quick Start in QEMU
   :id: SPEC_DEV_GUIDE_DEBUG_START
   :links: REQ_DEV_GUIDE_DEBUG_1, REQ_SYS_SIM_1
   :status: approved
   :tags: developer, documentation, debugging

   **Description:** The quick-start section SHALL provide the minimal steps
   to attach a GDB debugger via VS Code.

   **Required content:**

   - Launch QEMU debug mode (``run_qemu_debug.sh`` / VS Code task)
   - Start VS Code debug session (F5 / Run menu)
   - What VS Code does automatically (attach, load symbols)


.. spec:: Debugging Guide — Debugging Features
   :id: SPEC_DEV_GUIDE_DEBUG_FEATURES
   :links: REQ_DEV_GUIDE_DEBUG_1, SPEC_ARCH_LAYERS_1
   :status: approved
   :tags: developer, documentation, debugging

   **Description:** The debugging features section SHALL document the GDB
   capabilities available in the VS Code integration.

   **Required content:**

   - Breakpoints (how to set, conditional breakpoints)
   - Step-through (step over, into, out)
   - Variable inspection (watch window, hover)
   - Call stack navigation
   - Debug console (direct GDB commands)


docs/90_guides/web-flasher.rst
--------------------------------

.. spec:: Web Flasher Guide — Overview and Browser Requirements
   :id: SPEC_DEV_GUIDE_FLASH_OVERVIEW
   :links: REQ_DEV_GUIDE_FLASH_1, REQ_SYS_NET_1
   :status: approved
   :tags: developer, documentation, flasher

   **Description:** The overview section SHALL explain what the web flasher
   is and what browser / hardware prerequisites are needed.

   **Required content:**

   - What the web flasher does (browser-based, no Python/esptool needed)
   - Supported browsers and Web Serial API requirement
   - Hardware prerequisites (physical USB connection)


.. spec:: Web Flasher Guide — Flash Procedure
   :id: SPEC_DEV_GUIDE_FLASH_STEPS
   :links: REQ_DEV_GUIDE_FLASH_1, REQ_SYS_NET_1
   :status: approved
   :tags: developer, documentation, flasher

   **Description:** The usage guide section SHALL provide step-by-step
   instructions for flashing firmware via the web flasher.

   **Required content:**

   - Step 1: Build the firmware (``idf.py build``)
   - Step 2: Start the web server (``start-web-flasher.sh``)
   - Step 3: Open browser and connect device
   - Step 4: Select firmware files and flash
   - Troubleshooting pointer
