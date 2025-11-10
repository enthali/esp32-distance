Development Container Setup
=============================

This template uses containerized development with VS Code Dev Containers and GitHub Codespaces for consistent ESP-IDF development environment.

Development Options
-------------------

Recommended: GitHub Codespaces (Cloud)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

✅ No local setup required - works entirely in browser

✅ Pre-configured ESP-IDF v5.4.1 environment ready instantly

✅ USB device access through browser serial interface

✅ Perfect for trying the template before committing to hardware

Alternative: Local Dev Container
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Uses Docker container with ESP-IDF v5.4.1 pre-installed
- Requires Docker Desktop and VS Code with Dev Containers extension

.. note::
   This template focuses on Codespaces workflow. Local dev container support is provided but not officially supported.

Using the Dev Container
------------------------

In GitHub Codespaces
~~~~~~~~~~~~~~~~~~~~

1. Go to your GitHub repository
2. Click "**Code**" → "**Codespaces**" → "**Create codespace**"
3. Container will automatically build with ESP-IDF ready
4. Extensions (ESP-IDF, ESP-IDF Web) will be pre-installed

Local Development with Docker
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Ensure Docker Desktop is running
2. Open the project in VS Code
3. VS Code will prompt: "Reopen in Container" → Click **Yes**
4. Container builds automatically with ESP-IDF environment

Container Features
------------------

- **ESP-IDF v5.4.1**: Latest stable version pre-configured
- **Pre-configured VS Code**: ESP-IDF extension settings ready
- **Web Serial Support**: ESP-IDF Web extension for Codespaces flashing/monitoring
- **Port forwarding**: HTTP server (80, 443) and development ports
- **Privileged mode**: USB device access for flashing (local Docker only)

Build Commands in Container
----------------------------

.. code-block:: bash

   # Standard ESP-IDF commands work directly
   idf.py build
   idf.py flash monitor
   idf.py menuconfig

   # Container has ESP-IDF environment pre-loaded
   # No need to run export.sh or install.sh

Hardware Flashing
-----------------

- **Local Docker**: Hardware connected to host is accessible in container
- **Codespaces**: Use ESP-IDF Web extension for WebSerial/WebUSB flashing
- **Mixed workflow**: Build in container, flash on host if needed

Windows USB Device Setup (Local Dev Container Only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For Windows users running the dev container locally (not needed for Codespaces):

**Quick Setup**: Run ``tools/attach-esp32.ps1`` as Administrator - script automatically finds and attaches your ESP32.

**Manual Setup** (if script doesn't work):

1. Install usbipd-win: ``winget install usbipd``
2. Find device: ``usbipd list`` (look for USB-SERIAL CH340)
3. Bind device: ``usbipd bind --busid <BUSID>``
4. Attach to container: ``usbipd attach --wsl --busid <BUSID>``

.. note::
   GitHub Codespaces handles USB through browser serial interface - no manual setup needed.

Benefits
--------

✅ **Consistent environments** across Windows, macOS, Linux, Codespaces

✅ **No ESP-IDF setup** required for new team members

✅ **Isolated dependencies** - container changes don't affect host system

✅ **Easy onboarding** - clone repo, open in VS Code, start coding

✅ **Your existing setup unchanged** - host workflow still works

Container Configuration
-----------------------

The dev container is configured in ``.devcontainer/devcontainer.json``:

- Base image: ``espressif/idf:v5.4.1``
- VS Code extensions automatically installed
- Port forwarding for web development
- Git configuration inherited from host

Troubleshooting
---------------

**Issue**: Container fails to build

- **Solution**: Check Docker Desktop is running and has sufficient resources (4GB+ RAM recommended)

**Issue**: ESP32 device not visible in container

- **Solution**: For local Docker, ensure USB device is attached using usbipd (Windows) or passed through (Linux/macOS)

**Issue**: Extensions not installed

- **Solution**: Reload window (Ctrl+Shift+P → "Developer: Reload Window") or manually install from marketplace

**Issue**: Slow build times

- **Solution**: Dev container caches ESP-IDF, but first build always takes longer. Subsequent builds are faster.
