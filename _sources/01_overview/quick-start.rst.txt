Quick Start Guide
=================

Get your ESP32 Distance Sensor up and running in minutes! This guide covers both hardware and QEMU emulation paths.

Prerequisites
-------------

What You Need
~~~~~~~~~~~~~

**Minimum Requirements:**

* Web browser with serial port support (Chrome or Edge recommended)
* GitHub account (free tier sufficient)
* Internet connection

**For Hardware Development:**

* ESP32 development board
* HC-SR04 ultrasonic sensor
* WS2812 LED strip (40 LEDs)
* USB cable
* Jumper wires
* (Optional) External 5V power supply

**For Emulation Only:**

* Just a browser! QEMU runs everything in the cloud.

.. note::

   **First time with ESP32?** Don't worry! GitHub Codespaces handles all the complex setup automatically. No local installation required.

Path 1: QEMU Emulation (No Hardware)
-------------------------------------

Perfect for learning, testing, or remote workshops.

Step 1: Get the Code
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Fork or clone the repository
   git clone https://github.com/enthali/esp32-distance.git
   cd esp32-distance

Step 2: Open in Codespaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Click **Code** button on GitHub
2. Select **Codespaces** tab
3. Click **Create codespace on main** (or your branch)
4. Wait 2-5 minutes for initial setup (ESP-IDF installation)

.. note::

   **What's happening?** Codespaces is installing ESP-IDF v5.4.1, QEMU emulator, 
   and all development tools. This only happens once per codespace.

Step 3: Build the Project
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Build for QEMU target
   idf.py build

**Expected output:**

.. code-block:: text

   Project build complete. To flash, run:
   idf.py flash

**Build time:** ~30-60 seconds (incremental builds are much faster)

Step 4: Run in QEMU
~~~~~~~~~~~~~~~~~~~~

**Option A: Using VS Code Task (Recommended)**

1. Press ``Ctrl+Shift+P`` (or ``Cmd+Shift+P`` on Mac)
2. Type "Tasks: Run Task"
3. Select **"Run QEMU (No Debug)"**

**Option B: Using Command Line**

.. code-block:: bash

   # Start QEMU with network support
   ./tools/qemu/run_qemu.sh

**What you'll see:**

.. code-block:: text

   🚀 Starting ESP32 Distance Sensor in QEMU
   📡 Starting network bridge...
   🌐 HTTP proxy running on http://localhost:8080
   ✅ QEMU started successfully

   Distance Sensor Simulator:
   🟢🟢🟢⚪⚪⚪⚪⚪⚪⚪ (15cm)

Step 5: Access Web Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open your browser to:

.. code-block:: text

   http://localhost:8080

**Features available:**

* View current distance reading
* See LED strip visualization
* Access configuration interface (when implemented)

.. tip::

   **Port Forwarding:** GitHub Codespaces automatically forwards port 8080. 
   You can access it from your local browser even though QEMU runs in the cloud!

Step 6: Debug (Optional)
~~~~~~~~~~~~~~~~~~~~~~~~~

Want to set breakpoints and step through code?

1. Press ``F5`` in VS Code
2. Or use **Run and Debug** panel
3. Select **"Debug in QEMU"** configuration

**Debugging features:**

* Set breakpoints in code
* Step through execution line-by-line
* Inspect variables and memory
* View call stack and registers

Stop QEMU
~~~~~~~~~

.. code-block:: bash

   # Stop QEMU emulator
   ./tools/qemu/stop_qemu.sh

   # Or use VS Code task: "Stop QEMU"

Path 2: Hardware Development
-----------------------------

Build and flash to real ESP32 hardware.

Step 1: Assemble Hardware
~~~~~~~~~~~~~~~~~~~~~~~~~~

Follow the pin configuration from :doc:`hardware`:

**Connections:**

.. code-block:: text

   ESP32 → HC-SR04 Sensor:
     GPIO14 → Trig
     GPIO13 → Echo
     5V     → VCC
     GND    → GND

   ESP32 → WS2812 LED Strip:
     GPIO12 → DIN
     5V     → 5V/VCC
     GND    → GND

.. warning::

   **Power Check:** If using all 40 LEDs at high brightness, connect an external 
   5V power supply (3A+) to both ESP32 VIN and LED strip VCC. Share common ground!

Step 2: Get the Code
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Fork or clone the repository
   git clone https://github.com/enthali/esp32-distance.git
   cd esp32-distance

Step 3: Open in Codespaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Same as Path 1:

1. Click **Code** → **Codespaces** → **Create codespace**
2. Wait for ESP-IDF setup (~2-5 minutes first time)

Step 4: Connect ESP32
~~~~~~~~~~~~~~~~~~~~~~

1. Connect ESP32 to your computer via USB
2. In Codespaces, allow USB device access when prompted
3. Verify connection:

.. code-block:: bash

   # List available serial ports
   ls /dev/ttyUSB* /dev/ttyACM*

**Expected output:**

.. code-block:: text

   /dev/ttyUSB0  # Your ESP32 serial port

.. note::

   **Port name varies:** May be ``/dev/ttyUSB0``, ``/dev/ttyACM0``, or similar. 
   Use the port name you see in your system.

Step 5: Build the Project
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Build for ESP32 hardware
   idf.py build

**Build time:** ~30-60 seconds

Step 6: Flash to ESP32
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Flash and start monitoring serial output
   idf.py -p /dev/ttyUSB0 flash monitor

Replace ``/dev/ttyUSB0`` with your actual port name.

**What happens:**

1. **Erase flash** - Clears existing firmware
2. **Flash bootloader** - Low-level boot code
3. **Flash partition table** - Memory layout
4. **Flash application** - Your firmware
5. **Monitor serial** - Show boot logs and output

**Expected output:**

.. code-block:: text

   I (123) boot: ESP-IDF v5.4.1
   I (234) main: Starting ESP32 Distance Sensor
   I (345) wifi: WiFi initialized
   I (456) sensor: Distance sensor ready
   I (567) led: LED controller initialized
   🟢 Distance: 25cm

Step 7: Verify Operation
~~~~~~~~~~~~~~~~~~~~~~~~~

**Check LED Test Sequence:**

At boot, you should see LEDs cycle through:

1. Red sweep (left to right)
2. Green sweep (right to left)
3. Blue sweep (left to right)
4. All LEDs off

**Check Distance Measurement:**

* Move hand in front of sensor (10-50cm range)
* LED position should track hand movement
* Green LED = normal range
* Red LED = out of range

**Check WiFi:**

.. code-block:: text

   I (2000) wifi: AP started
   I (2100) wifi: SSID: ESP32-Distance-Sensor
   I (2200) wifi: Connect to configure

1. Look for WiFi network "ESP32-Distance-Sensor"
2. Connect (no password required)
3. Browser should open captive portal automatically
4. If not, navigate to ``http://192.168.4.1``

Step 8: Monitor Serial Output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**To exit monitor:** Press ``Ctrl+]``

**Useful monitor commands:**

* ``Ctrl+T`` → ``Ctrl+R`` : Reset ESP32
* ``Ctrl+T`` → ``Ctrl+H`` : Show help

**To monitor without flashing:**

.. code-block:: bash

   idf.py -p /dev/ttyUSB0 monitor

Common Issues & Troubleshooting
--------------------------------

Build Errors
~~~~~~~~~~~~

**Error: "idf.py not found"**

* ESP-IDF not sourced in current shell
* In Codespaces, this should be automatic
* Try opening a new terminal

**Error: "Component not found"**

* Check CMakeLists.txt in main/ and components/
* Verify all component directories exist
* Run ``idf.py fullclean`` then rebuild

Flash Errors
~~~~~~~~~~~~

**Error: "Serial port not found"**

* ESP32 not connected or detected
* Check USB cable (must support data, not just power)
* Try different USB port
* Check device permissions: ``ls -l /dev/ttyUSB0``

**Error: "Failed to connect to ESP32"**

* Hold BOOT button while connecting
* Press RESET button after ``Connecting...`` message
* Check USB cable and connection

Hardware Issues
~~~~~~~~~~~~~~~

**LEDs not lighting:**

* Check power supply (USB may be insufficient for 40 LEDs)
* Verify GPIO12 connection to DIN
* Test with fewer LEDs first
* Check LED strip polarity (DIN, not DOUT)

**Sensor not responding:**

* Verify GPIO13/14 connections
* Check sensor power (VCC=5V, GND=GND)
* Ensure nothing blocking sensor (remove tape, plastic)
* Test sensor range (2cm-400cm specification)

**WiFi not starting:**

* Check serial output for errors
* Verify antenna is connected (if using external antenna)
* Try different WiFi channel (default is auto)

QEMU Issues
~~~~~~~~~~~

**QEMU won't start:**

* Another QEMU instance may be running: ``./tools/qemu/stop_qemu.sh``
* Check port conflicts (5555, 5556, 8080)
* Review console output for errors

**Web interface not accessible:**

* Verify HTTP proxy is running (should auto-start)
* Check port forwarding in Codespaces
* Try direct access: ``curl http://192.168.100.2``

Next Steps
----------

Now that your system is running:

1. **Explore the Code** - Check out ``main/main.c`` and component sources
2. **Read Requirements** - See :doc:`../11_requirements/index` for specifications
3. **Understand Design** - Review :doc:`../12_design/index` for architecture
4. **Join Workshop** - (Coming soon) Hands-on exercises and challenges
5. **Customize** - Add features, change LED patterns, implement color zones

Useful Commands
---------------

**Build & Flash:**

.. code-block:: bash

   # Clean build
   idf.py fullclean
   idf.py build

   # Flash only (no rebuild)
   idf.py -p /dev/ttyUSB0 flash

   # Build, flash, and monitor in one command
   idf.py -p /dev/ttyUSB0 build flash monitor

**Configuration:**

.. code-block:: bash

   # Open menuconfig (TUI configuration)
   idf.py menuconfig

   # Set target (if needed)
   idf.py set-target esp32

**Analysis:**

.. code-block:: bash

   # Check memory usage
   idf.py size

   # Check memory details by component
   idf.py size-components

**QEMU:**

.. code-block:: bash

   # Run QEMU (no debug)
   ./tools/qemu/run_qemu.sh

   # Run QEMU with debug server
   ./tools/qemu/run_qemu_debug.sh

   # Stop QEMU
   ./tools/qemu/stop_qemu.sh

   # View UART0 output
   ./tools/qemu/uart0_terminal.sh

Getting Help
------------

**Documentation:**

* :doc:`../90_guides/index` - Development guides (QEMU, debugging, devcontainer)
* :doc:`../11_requirements/index` - System requirements
* :doc:`../12_design/index` - Design specifications
* :doc:`../21_api/index` - API reference

**Community:**

* `GitHub Issues <https://github.com/enthali/esp32-distance/issues>`_ - Report bugs or ask questions
* `GitHub Discussions <https://github.com/enthali/esp32-distance/discussions>`_ - General questions and ideas

**ESP-IDF Resources:**

* `ESP-IDF Documentation <https://docs.espressif.com/projects/esp-idf/en/latest/>`_
* `ESP32 Technical Reference <https://www.espressif.com/en/support/documents/technical-documents>`_

Happy Building! 🚀
------------------

You're now ready to explore, learn, and customize the ESP32 Distance Sensor project. Have fun experimenting!
