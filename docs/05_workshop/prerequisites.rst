Workshop Prerequisites
======================

Before starting the workshop, ensure you have the required software accounts, browser support, and (optionally) hardware components.

Software & Licenses
-------------------

Supported Browser
~~~~~~~~~~~~~~~~~

To use browser-based serial port access and the ESP-IDF Web extension, you must use a browser that supports the **Web Serial API**.

**Supported Browsers:**

* Google Chrome (latest version)
* Microsoft Edge (latest version)
* Opera (latest version)
* Arc Browser (latest version)

.. warning::

   **Not Supported:** Firefox, Safari, and most mobile browsers do **not** support 
   Web Serial API and cannot be used for direct device communication.

**Recommendation:** Use **Chrome** or **Edge** on desktop for the best experience.

GitHub Account
~~~~~~~~~~~~~~

You will need a GitHub account to participate in the workshop.

**Account Options:**

* **Free Account** - Sufficient for all workshop activities
* **Pro Account** - Provides additional Codespaces hours (optional)

**What you can do:**

* Access the repository and documentation
* Fork and clone the project
* Use GitHub Codespaces (within free usage quota)
* Collaborate with other participants

**Sign Up:** If you don't have an account, create one at `github.com <https://github.com>`_ (free).

.. note::

   **Codespaces Free Tier:** GitHub provides 60 core-hours per month free for Codespaces. 
   This workshop typically uses 2-4 core-hours, so the free tier is more than sufficient.

GitHub Copilot (Recommended)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

GitHub Copilot is an AI-powered coding assistant that helps you write code faster and with fewer errors by providing real-time code suggestions, explanations, and even generating entire functions.

**Copilot Tiers:**

* **Copilot Free** - Basic code completion and chat (sufficient for most workshop tasks)
* **Copilot Pro** - Advanced features including coding agent, multi-file editing
* **Copilot Business/Enterprise** - For organizations

**What Copilot Provides:**

* Real-time code suggestions as you type
* Code explanations and documentation
* Error troubleshooting and fixes
* Function and file generation
* Interactive coding assistance

**For This Workshop:**

* **Free Tier:** Sufficient for basic code suggestions and chat
* **Pro Tier:** Recommended for coding agent and advanced features
* **30-Day Free Trial:** Available for Copilot Pro (no payment required during trial)

.. tip::

   **Try Copilot Pro Free:** Sign up for the 30-day free trial to access the full 
   coding agent during the workshop. You can explore advanced features without any cost.

GitHub Codespaces
~~~~~~~~~~~~~~~~~

GitHub Codespaces provides a cloud-based development environment pre-configured for this project.

**Benefits:**

* **Zero Setup** - Start coding instantly in your browser or VS Code
* **Pre-configured** - ESP-IDF, toolchain, and libraries already installed
* **Consistent Environment** - Same setup for all participants
* **No Local Install** - Works on any OS (Windows, Mac, Linux, Chromebook)

**What's Included:**

* ESP-IDF v5.4.1
* QEMU emulator for hardware simulation
* All required build tools and dependencies
* VS Code with ESP-IDF extensions
* UART terminal access for serial monitoring

.. note::

   **Everything else is provided inside the preconfigured Codespaces environment.** 
   No manual installation required!

Hardware Requirements
---------------------

Physical hardware is **optional** for this workshop. You can complete most exercises using QEMU emulation. However, for hands-on experience with real sensors and LEDs, you'll need the following components.

Required Hardware (For Physical Testing)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ESP32 Development Board
^^^^^^^^^^^^^^^^^^^^^^^

**Recommended:** ESP32 WROOM-32F (4MB flash)

**Specifications:**

* WiFi and Bluetooth connectivity
* Multiple GPIO pins for sensors and LEDs
* USB interface for programming
* Compatible with ESP-IDF v5.4.1

**Alternative ESP32 Variants:**

Most ESP32 boards will work, but pin assignments and memory constraints may vary. The workshop is optimized for ESP32-WROOM-32 with 4MB flash.

**What's Usually Included:**

Most ESP32 development boards include a built-in USB-to-serial adapter (CP2102, CH340, or FTDI chip) for programming and monitoring.

HC-SR04 Ultrasonic Distance Sensor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Purpose:** Measures distance for the sensor project.

**Specifications:**

* Measurement range: 2cm - 400cm
* Operating voltage: 5V (logic levels compatible with ESP32)
* Interface: Digital trigger and echo pins

**Connections:**

* Trigger pin → ESP32 GPIO14
* Echo pin → ESP32 GPIO13
* VCC → 5V
* GND → Ground

WS2812 Addressable LED Strip
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Purpose:** Visual display of distance measurements.

**Recommended:** 40 LEDs (configurable in software)

**Specifications:**

* Individually addressable RGB LEDs
* 5V power supply
* Single-wire data interface
* Current draw: ~60mA per LED at full brightness

**Connection:**

* Data IN (DIN) → ESP32 GPIO12
* VCC → 5V power supply
* GND → Ground

.. warning::

   **Power Considerations:** 40 LEDs at full brightness can draw up to 2.4A. 
   USB power (typically 500-900mA) may be insufficient. Consider using an external 
   5V power supply rated for at least 3A.

USB Cable and Power
^^^^^^^^^^^^^^^^^^^

**For Programming:**

* USB cable (Micro-USB or USB-C, depending on your ESP32 board)
* Must support data transfer (not just charging)

**For LED Strip (Optional):**

* External 5V power supply (3A+ recommended)
* Only needed if using many LEDs at high brightness
* USB power sufficient for testing with lower brightness

USB Serial Adapter (If Needed)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Usually Not Required:** Most ESP32 development boards include a built-in USB-to-serial adapter.

**If your board doesn't have one:**

* FTDI adapter (FT232RL)
* CP2102 USB-to-UART bridge
* CH340 USB-to-serial converter

**Used for:**

* Uploading firmware
* Debugging via serial monitor
* Monitoring ESP32 output

Optional Hardware
~~~~~~~~~~~~~~~~~

**For Enhanced Experience:**

* Breadboard for prototyping
* Jumper wires (male-to-male, male-to-female)
* External 5V power supply (3A+)
* Mounting hardware (for permanent installation)
* Enclosure (for finished project)

Hardware-Optional Workshop Path
--------------------------------

Don't have hardware yet? No problem!

QEMU Emulation Mode
~~~~~~~~~~~~~~~~~~~~

The workshop fully supports QEMU emulation, allowing you to:

* Complete all coding exercises
* Test sensor simulation (animated distance readings)
* View LED visualization (emoji-based terminal output)
* Access web interface via HTTP proxy
* Debug with GDB breakpoints

**What Works in QEMU:**

✅ Distance sensor simulation (5cm→60cm animated sweep)  
✅ LED strip visualization (terminal-based with emojis)  
✅ WiFi and network stack (via UART tunnel)  
✅ Web server and configuration interface  
✅ All software functionality

**What Requires Hardware:**

❌ Physical sensor readings (use simulator instead)  
❌ Physical LED strip (use terminal visualization)  
❌ Flash to real ESP32 board

.. tip::

   **Recommended Path:** Start with QEMU emulation to learn the software, 
   then add physical hardware later for the complete experience!

Software Installation (Local Development)
------------------------------------------

.. note::

   **Not required for Codespaces!** This section is only for participants who 
   want to develop locally instead of using GitHub Codespaces.

For Local Development (Optional):

* Docker Desktop (for VS Code Dev Containers)
* VS Code with Dev Containers extension
* Git for version control

**OR:**

* ESP-IDF v5.4.1 (manual installation)
* Python 3.8+ with required packages
* Build tools (CMake, Ninja, etc.)

.. warning::

   **Local setup is complex!** We strongly recommend using GitHub Codespaces 
   instead. Local installation can take 1-2 hours and may have compatibility 
   issues across different operating systems.

Preparation Checklist
---------------------

Before Workshop Day:

.. code-block:: text

   □ Create or verify GitHub account
   □ Install Chrome or Edge browser
   □ (Optional) Sign up for Copilot Pro free trial
   □ (Optional) Assemble hardware components
   □ (Optional) Test USB cable with ESP32

On Workshop Day:

.. code-block:: text

   □ Open supported browser (Chrome/Edge)
   □ Log in to GitHub
   □ Fork the workshop repository
   □ Create a Codespace (wait ~5 minutes for setup)
   □ Verify Codespace loads successfully

.. tip::

   **Test Early:** Create your Codespace a day before the workshop to ensure 
   everything works. This avoids any last-minute setup issues!

Troubleshooting
---------------

Common Setup Issues
~~~~~~~~~~~~~~~~~~~

**Browser Not Supported:**

* Solution: Install Chrome or Edge (latest version)
* Alternative: Use QEMU emulation only (no serial port needed)

**GitHub Account Issues:**

* Verify email address is confirmed
* Check Codespaces usage quota (Settings → Billing)
* Try incognito/private browsing mode

**Copilot Not Working:**

* Verify Copilot subscription is active
* Reload VS Code window (Command Palette → "Reload Window")
* Check Copilot status in VS Code status bar

**Codespace Won't Start:**

* Check GitHub status page for service issues
* Try deleting and recreating the Codespace
* Verify you haven't exceeded free tier limits

Hardware Setup Issues
~~~~~~~~~~~~~~~~~~~~~

**ESP32 Not Recognized:**

* Try different USB cable (must support data)
* Install USB-to-serial drivers (CH340, CP2102)
* Check Device Manager (Windows) or ``lsusb`` (Linux)

**Insufficient Power:**

* Use external 5V power supply for LED strip
* Reduce LED brightness in software
* Test with fewer LEDs first

Getting Help
------------

**Before the Workshop:**

* Review :doc:`../01_overview/quick-start` for general setup
* Check `GitHub Issues <https://github.com/enthali/esp32-distance/issues>`_
* Test your Codespace early

**During the Workshop:**

* Ask facilitator (if in guided workshop)
* Use GitHub Copilot for immediate assistance
* Check troubleshooting sections in guides
* Collaborate with other participants

Next Steps
----------

Prerequisites ready? Move on to:

* :doc:`hardware-setup` - Assemble your components (if using hardware)
* :doc:`getting-started` - Fork, build, and run the project
* :doc:`exercises` - Complete the workshop challenges

---

**Ready to start?** Let's build something awesome! 🚀
