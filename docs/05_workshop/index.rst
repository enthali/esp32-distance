Workshop Overview
=================

.. image:: ../_static/images/setup.jpg
   :alt: ESP32 Distance Sensor Workshop Setup
   :align: center
   :width: 600px

Welcome to the ESP32 Distance Sensor Workshop! This hands-on workshop is designed to get you coding on real hardware in **just 5 minutes**—no complicated setup, no frustration! 

Well at least thats the goal 😉, if you experience issues, feel free to contact us.

.. toctree::
   :maxdepth: 2
   :caption: Workshop Materials:

   prerequisites
   getting-started
   hardware-setup
   exercises

Quick Start Path
----------------

If you have a GitHub account and the required hardware, you can:

1. **Fork this project** from `GitHub <https://github.com/enthali/esp32-distance>`_
2. **Create a new Codespace** (just a few clicks)
3. **Wait ~5 minutes** for setup (all tools & dependencies installed automatically)
4. **Activate ESP-IDF Web plugin** (required for direct hardware connection)
5. **Start coding and flashing** your ESP32—productive immediately!

.. note::

   **It's really that easy:** From zero to coding on real hardware in minutes. 
   No local installs, no driver hassle, no toolchain pain.

What You Need
-------------

Minimum Requirements:

* **GitHub account** (free or pro tier)
* **Required hardware** - See :doc:`hardware-setup` for details
* **Supported browser** - Check :doc:`prerequisites` for browser requirements

**Optional but recommended:**

* Physical ESP32 board, sensor, and LED strip (can start with QEMU emulation)
* GitHub Copilot access (enhances learning experience)

Getting Started
---------------

New to the workshop? Start here:

1. **Check Prerequisites** - Review :doc:`prerequisites` to ensure your browser and account are ready
2. **Get Hardware Ready** - Follow :doc:`hardware-setup` for component assembly
3. **Begin Workshop** - Work through :doc:`getting-started` step-by-step
4. **Complete Exercises** - Try the challenges in :doc:`exercises`

.. tip::

   **No hardware yet?** Start with QEMU emulation! You can complete most of the workshop 
   using the emulator, then add hardware later. See the main :doc:`../01_overview/quick-start` 
   guide for QEMU setup.

What You'll Learn
-----------------

This workshop covers modern embedded development practices:

**Embedded Systems Fundamentals:**

* ESP32 microcontroller architecture
* FreeRTOS real-time operating system
* Hardware interfaces (GPIO, UART, RMT)
* Sensor integration (HC-SR04 ultrasonic)
* LED control (WS2812 addressable LEDs)

**IoT Connectivity:**

* WiFi networking (AP and STA modes)
* HTTP web server implementation
* Captive portal for configuration
* Configuration persistence (NVS storage)

**Modern Development Tools:**

* GitHub Codespaces for cloud development
* ESP-IDF official development framework
* QEMU emulation for hardware-optional testing
* GDB debugging with breakpoints
* GitHub Copilot for AI-assisted coding

**Software Engineering:**

* Component-based architecture
* Requirements engineering with Sphinx-Needs
* Professional documentation practices
* Version control with Git
* Testing and validation

Workshop Structure
------------------

The workshop is organized into progressive sections:

**Part 1: Setup & Basics** (30 minutes)

* Environment setup with Codespaces
* First build and flash
* Understanding the project structure
* Basic sensor and LED testing

**Part 2: Understanding the Code** (45 minutes)

* Component architecture walkthrough
* FreeRTOS task management
* Sensor reading and processing
* LED visualization logic

**Part 3: Customization** (60 minutes)

* Modify LED patterns and colors
* Implement color zones (red/yellow/green)
* Adjust distance ranges
* Add custom features

**Part 4: Advanced Topics** (Optional, 45 minutes)

* Web interface customization
* Configuration management
* QEMU emulation and debugging
* Requirements traceability

**Total Workshop Time:** 2-3 hours (self-paced)

Workshop Benefits
-----------------

Why This Workshop?
~~~~~~~~~~~~~~~~~~

**Zero-Setup Development:**

* No local installation required
* Supports local defcontainer setup if codespaces is not an option
* Consistent environment for all participants
* Works on Windows, Mac, Linux, Chromebook

**Hands-On Learning:**

* Real hardware interaction (or QEMU simulation)
* Immediate feedback with LED visualization
* Practical IoT device with real-world application
* Build something you can actually use!

**Professional Practices:**

* Industry-standard tools (ESP-IDF, FreeRTOS)
* Modern development workflow (Codespaces, Git)
* Requirements engineering methodology
* Component-based architecture

**AI-Assisted Development:**

* GitHub Copilot integration
* Learn how AI helps with embedded development
* Practical examples of AI code generation
* Troubleshooting with AI assistance

Workshop Tips
-------------

**Use GitHub Copilot:**

If you have Copilot access, use it throughout the workshop:

* Ask Copilot Chat for explanations of ESP32 concepts
* Get code suggestions for modifications
* Troubleshoot errors with AI assistance
* Learn ESP-IDF APIs interactively

**Work at Your Own Pace:**

* All materials are self-paced
* Take breaks when needed
* Experiment and explore beyond the exercises
* Don't worry about breaking things (easy to reset!)

**Ask for Help:**

* Workshop materials include troubleshooting sections
* Check the main documentation for detailed guides
* Open GitHub issues for specific problems
* Join discussions for general questions
* And if it's a handon session, ask the facilitator!

**Have Fun:**

* This is about learning and experimenting
* Try different LED patterns and colors
* Customize the project to your needs
* Share your creations with the community!

Prerequisites & Setup
---------------------

Before starting the workshop:

1. **Prerequisites** (:doc:`prerequisites`)

   * GitHub account setup
   * Browser requirements
   * Hardware checklist (if using physical devices)

2. **Hardware Setup** (:doc:`hardware-setup`)

   * Component identification
   * Wiring and connections
   * Power considerations
   * Testing hardware

3. **Getting Started** (:doc:`getting-started`)

   * Fork and clone repository
   * Create Codespace
   * Build and flash
   * First test run

Workshop Exercises
------------------

Once you're set up, work through the exercises:

* :doc:`exercises` - Hands-on challenges and learning activities

**Exercise Topics:**

* Modify LED colors and patterns
* Implement distance-based color zones
* Add sound alerts (if using buzzer)
* Customize web interface
* Create your own features

Getting Help
------------

**During the Workshop:**

* Check troubleshooting sections in each guide
* Use GitHub Copilot for immediate assistance
* Review the main documentation at :doc:`../index`
* Ask workshop facilitator (if in guided workshop)

**After the Workshop:**

* `GitHub Issues <https://github.com/enthali/esp32-distance/issues>`_ - Report bugs or ask questions
* `GitHub Discussions <https://github.com/enthali/esp32-distance/discussions>`_ - General questions and ideas
* Community support - Share your projects and help others!

**Documentation Resources:**

* :doc:`../01_overview/index` - Project overview and introduction
* :doc:`../11_requirements/index` - System requirements
* :doc:`../12_design/index` - Design specifications
* :doc:`../90_guides/index` - Development guides (QEMU, debugging)

Next Steps
----------

Ready to begin? Start with:

1. :doc:`prerequisites` - Check browser and account requirements
2. :doc:`hardware-setup` - Prepare your hardware (or plan for QEMU)
3. :doc:`getting-started` - Fork, build, and flash!
4. :doc:`exercises` - Complete the challenges

.. note::

   **Remote Workshop?** All materials work perfectly with QEMU emulation. 
   Participants can follow along without any hardware, then add physical 
   devices later if desired.

---

**Let's build something awesome together!** 🚀

**Have fun, experiment, and happy coding!** 🎉
