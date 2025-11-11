Project Introduction
====================

How This Project Was Born (Or: The Garage Door Incident)
---------------------------------------------------------

Picture this: You're driving your car into your garage after a long day. You know that sweet spot exists—where your car is fully inside, but you haven't kissed the wall with your bumper. But where is it?

You inch forward... *a little more*... *just a bit further*... **CRUNCH!** 🤦

Or worse: You play it safe, stop too early, and the garage door guillotines your trunk.

Sound familiar? That's exactly how this project started. One frustrated evening, one slightly dented bumper, and one thought: *"There has to be a better way!"*

The Solution
------------

Enter the ESP32 Distance Sensor: Your garage's new best friend. An ultrasonic sensor measures the distance, and a LED strip lights up to show you exactly where you are.

**Right now**, you'll see a single 🟢 **green LED** moving along the strip as you get closer or further away—simple, effective, and it works!

**In the workshop**, we'll show you how to customize it: Want color zones? (🔴 Red for "STOP!", 🟡 Yellow for "careful...", 🟢 Green for "perfect spot"?) That's what we'll build together!

Because why solve one problem when you can over-engineer it beautifully? 🎉

What Makes This Project Special?
---------------------------------

Real-World Application
~~~~~~~~~~~~~~~~~~~~~~

This isn't just a blinking LED demo. It's a practical IoT device that solves a real problem many people face. You'll learn embedded development while building something genuinely useful.

Modern Development Practices
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **GitHub Codespaces** - Cloud-based development, no local setup required
* **QEMU Emulation** - Test without hardware, includes full network stack
* **Requirements Engineering** - Professional Sphinx-Needs methodology
* **Component Architecture** - Clean, modular, maintainable code structure

Hardware-Optional Learning
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Don't have an ESP32 or sensors yet? No problem! The project includes:

* **Distance Sensor Simulator** - Animated distance readings (5cm→60cm sweep)
* **LED Visualizer** - Terminal-based display with emoji blocks 🔴🟢🔵
* **Network Emulation** - Full TCP/IP stack via UART tunnel
* **Web Interface** - HTTP server accessible from your browser

You can learn, code, and test everything in QEMU before ordering any hardware.

Workshop-Ready
~~~~~~~~~~~~~~

This project is designed for hands-on workshops:

* **5-minute setup** with GitHub Codespaces
* **No driver issues** or toolchain problems
* **Consistent environment** for all participants
* **Remote-friendly** with QEMU emulation
* **Professional practices** from day one

What You'll Learn
-----------------

Embedded Systems Fundamentals
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **Real-time Operating System (FreeRTOS)** - Task management and concurrency
* **Hardware Interfaces** - GPIO, UART, RMT (for LED timing)
* **Sensor Integration** - HC-SR04 ultrasonic distance measurement
* **Memory Management** - Heap and stack usage on constrained devices

IoT Connectivity
~~~~~~~~~~~~~~~~

* **WiFi Networking** - AP and STA modes, captive portal
* **HTTP Web Server** - Mobile-responsive user interface
* **Configuration Storage** - NVS (Non-Volatile Storage) for persistence
* **Network Protocols** - TCP/IP, DNS, HTTP

Software Engineering
~~~~~~~~~~~~~~~~~~~~

* **Component-Based Architecture** - Modular design with clean interfaces
* **Requirements Engineering** - Sphinx-Needs for traceability
* **Version Control** - Git workflow and collaboration
* **Documentation** - Technical writing and API documentation
* **Testing** - QEMU emulation and hardware validation

Modern Development Tools
~~~~~~~~~~~~~~~~~~~~~~~~

* **GitHub Codespaces** - Cloud-based development environment
* **ESP-IDF** - Official Espressif IoT Development Framework
* **QEMU** - Full system emulation for testing
* **GDB Debugging** - Step through code with breakpoints
* **GitHub Copilot** - AI-assisted coding and learning

Project Goals
-------------

1. **Solve a Real Problem** - Help people park their cars without mishaps
2. **Learn Modern Embedded Development** - Professional practices and tools
3. **Build Practical Skills** - Hardware, software, networking, and debugging
4. **Have Fun!** - Experiment, customize, and see your code light up (literally!)

Next Steps
----------

Ready to dive in? Here's your path forward:

1. **Understand the Hardware** - See :doc:`hardware` for requirements and pin configuration
2. **Get Started** - Follow :doc:`quick-start` to build and flash the project
3. **Explore the Code** - Check out the component architecture in ``main/components/``
4. **Join the Workshop** - (Coming soon) Hands-on exercises and challenges
5. **Customize It** - Make it your own with color zones, sounds, or other features!

.. note::

   **No hardware yet?** Start with QEMU emulation! You can build, test, and learn everything without physical devices. See the `QEMU Emulator Guide <../90_guides/qemu-emulator.html>`_ for details.

Have Fun!
---------

This project is about learning, experimenting, and building something cool. Don't worry if things don't work perfectly the first time—that's part of the journey!

If you get stuck, check the documentation, ask questions, or open an issue. This is a friendly space for makers, learners, and tinkerers of all levels.

**Let's build something awesome together!** 🚀
