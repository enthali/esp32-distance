Development Guide
=================

This guide covers all aspects of developing with the ESP32 Distance Sensor, from setting up your environment to debugging and testing.

.. toctree::
   :maxdepth: 2
   :caption: Development Topics:

   devcontainer
   pre-commit-hooks
   qemu-emulator
   web-flasher
   debugging
   qemu-network-internals
   known-issues

Getting Started
---------------

Recommended: GitHub Codespaces
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The fastest way to start developing:

1. Click "**Use this template**" to create your repository
2. Open in **GitHub Codespaces** (Code → Codespaces → Create)
3. Wait for container to build (first time only, ~2 minutes)
4. Start coding in ``main/main.c``

Testing Without Hardware
~~~~~~~~~~~~~~~~~~~~~~~~

Use QEMU emulation:

.. code-block:: bash

   # Build the project
   idf.py build

   # Run in QEMU with network support
   ./tools/run-qemu-network.sh

   # Debug in QEMU
   # Press F5 in VS Code or use Debug panel

Flashing Real Hardware
~~~~~~~~~~~~~~~~~~~~~~

When you have physical ESP32:

.. code-block:: bash

   # Build the project
   idf.py build

   # Flash and monitor (replace /dev/ttyUSB0 with your port)
   idf.py -p /dev/ttyUSB0 flash monitor

   # Exit monitor: Press Ctrl+]

Development Workflow
--------------------

Typical development cycle:

.. code-block:: bash

   # 1. Customize application
   vim main/main.c

   # 2. Build
   idf.py build

   # 3. Test in QEMU (optional)
   ./tools/run-qemu-network.sh

   # 4. Flash to hardware
   idf.py -p /dev/ttyUSB0 flash monitor

Template Benefits
-----------------

- **Consistent Environment**: Same toolchain everywhere (Codespaces, Docker, CI/CD)
- **ESP-IDF v5.4.1**: Latest stable version pre-configured
- **QEMU Support**: Test without hardware using full network emulation
- **GDB Debugging**: Full breakpoint debugging in emulator
- **Web Interface**: Access emulated web server via browser
- **Documentation**: Professional requirements engineering with Sphinx-Needs

Next Steps
----------

- **New to dev containers?** Start with :doc:`devcontainer`
- **Want to debug?** Check out :doc:`debugging`
- **Curious about QEMU networking?** See :doc:`qemu-emulator`
- **Need technical details?** Read :doc:`qemu-network-internals`

Known Issues
------------

.. note::
   ESP-IDF-WEB extension doesn't automatically install in Codespaces. 
   Please search for "ESP-IDF Web" in the VS Code marketplace and install it manually.
