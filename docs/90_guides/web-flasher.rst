Web-Based Firmware Flashing
===========================

Flash your ESP32 directly from your browser using the Web Serial API - no Python, no esptool, no command-line complexity.

Overview
--------

The web flasher provides a browser-based interface for flashing firmware to ESP32 devices. This is especially useful when:

- Working in GitHub Codespaces or remote development containers
- USB passthrough to containers is not available
- You want a simple one-click flashing experience
- You need to flash from environments without Python/esptool

Quick Start
-----------

1. **Build firmware:**

   .. code-block:: bash

      idf.py build

2. **Start web flasher server:**

   .. code-block:: bash

      ./tools/web-flasher/start-web-flasher.sh

3. **Forward port 8001** in VS Code (Ports tab)

4. **Open the forwarded URL** in Chrome/Edge/Opera

5. **Click "Connect and Flash ESP32"** and select your device

Features
--------

- ✅ **Browser-based** - No installation required
- ✅ **One-click operation** - Flash bootloader, partition table, and application
- ✅ **Serial monitor** - View ESP32 output in browser
- ✅ **Progress tracking** - Real-time flashing status
- ✅ **Codespaces compatible** - Works in remote environments

Browser Requirements
--------------------

Supported Browsers
~~~~~~~~~~~~~~~~~~

The web flasher requires the `Web Serial API <https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API>`_:

- **Chrome** 89+
- **Edge** 89+
- **Opera** 75+

.. warning::
   
   Firefox and Safari do not yet support the Web Serial API.

Hardware Requirements
~~~~~~~~~~~~~~~~~~~~~

- ESP32 connected via USB to your **local computer** (not the container)
- USB cable with data lines (not charge-only cable)

Usage Guide
-----------

Step 1: Build Firmware
~~~~~~~~~~~~~~~~~~~~~~~

Build your project first:

.. code-block:: bash

   cd /workspaces/esp32-template  # Or your project directory
   idf.py build

This generates the required binary files:

- ``build/bootloader/bootloader.bin``
- ``build/partition_table/partition-table.bin``
- ``build/your-project-name.bin``

Step 2: Start Web Server
~~~~~~~~~~~~~~~~~~~~~~~~~

Run the web flasher server:

.. code-block:: bash

   ./tools/web-flasher/start-web-flasher.sh

This automatically:

- Generates ``build/flasher-manifest.json`` with flash offsets
- Starts HTTP server on port 8001
- Serves the web flasher interface

Step 3: Port Forwarding
~~~~~~~~~~~~~~~~~~~~~~~~

In VS Code (GitHub Codespaces):

1. Open the **Ports** tab (next to Terminal)
2. Click **Forward a Port**
3. Enter ``8000``
4. Set visibility to **Public** if accessing from different network
5. Click on the forwarded URL to open in browser

Step 4: Connect and Flash
~~~~~~~~~~~~~~~~~~~~~~~~~~

In the browser:

1. Connect ESP32 via USB to your **local computer**
2. Click **"Connect and Flash ESP32"**
3. Select your ESP32's serial port from the browser dialog
4. Click **Install** to start flashing

The flasher will:

- Erase flash (if needed)
- Flash bootloader at offset 0x1000
- Flash partition table at offset 0x8000
- Flash application at offset 0x10000
- Show real-time progress

Step 5: Monitor Output
~~~~~~~~~~~~~~~~~~~~~~

After flashing:

- ✅ "Installation successful" message appears
- 📡 Serial console shows ESP32 output
- 🔄 Option to reconnect or flash again

Architecture
------------

How It Works
~~~~~~~~~~~~

.. code-block:: text

   ┌─────────────────┐
   │  GitHub         │
   │  Codespaces     │
   │  Container      │
   │                 │
   │  ┌───────────┐  │
   │  │ HTTP      │  │  Port 8001 forwarded
   │  │ Server    │◄─┼──────────────────────┐
   │  └───────────┘  │                      │
   │                 │                      │
   │  ┌───────────┐  │                      │
   │  │ Firmware  │  │                      ▼
   │  │ Binaries  │  │              ┌──────────────┐
   │  └───────────┘  │              │   Browser    │
   └─────────────────┘              │  (Chrome)    │
                                    └──────┬───────┘
                                           │ Web Serial API
                                           │
                                    ┌──────▼───────┐
                                    │    USB       │
                                    └──────┬───────┘
                                           │
                                    ┌──────▼───────┐
                                    │    ESP32     │
                                    └──────────────┘

Components
~~~~~~~~~~

1. **HTTP Server** (``flasher_server.py``)
   
   - Serves web interface and firmware files
   - Provides manifest.json with flash offsets

2. **Web Interface** (``web-flasher.html``)
   
   - Built on ESP Web Tools library
   - Handles Web Serial API communication
   - Provides UI for connection and monitoring

3. **Manifest Generator** (``generate-flasher-manifest.sh``)
   
   - Scans build directory for binaries
   - Generates ESP Web Tools compatible manifest
   - Extracts project name from CMakeLists.txt

Manifest Format
~~~~~~~~~~~~~~~

The ``build/flasher-manifest.json`` tells the browser what to flash:

.. code-block:: json

   {
     "name": "your-project-name",
     "version": "1.0.0",
     "builds": [
       {
         "chipFamily": "ESP32",
         "parts": [
           {
             "path": "../build/bootloader/bootloader.bin",
             "offset": 4096
           },
           {
             "path": "../build/partition_table/partition-table.bin",
             "offset": 32768
           },
           {
             "path": "../build/your-project-name.bin",
             "offset": 65536
           }
         ]
       }
     ]
   }

Troubleshooting
---------------

Port 8001 Already in Use
~~~~~~~~~~~~~~~~~~~~~~~~~

**Symptom:** Server fails to start with "Address already in use"

**Solution:**

.. code-block:: bash

   # Stop any running web flasher
   ./tools/web-flasher/stop-web-flasher.sh
   
   # Or manually kill process
   pkill -f flasher_server.py
   lsof -ti:8001 | xargs kill -9

Serial Port Not Appearing
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Symptom:** Browser dialog shows no serial ports

**Possible Causes:**

1. **ESP32 not connected** - Check USB connection to local computer
2. **Wrong cable** - Ensure cable has data lines (not charge-only)
3. **Driver missing** - Install CH340/CP2102 USB-to-serial drivers
4. **Browser permission** - Allow serial port access in browser

**Solution:**

- Try different USB port
- Restart ESP32 (press reset button)
- Check Device Manager (Windows) / ``ls /dev/tty*`` (Mac/Linux)
- Grant browser serial port permissions

Flash Failed or Timeout
~~~~~~~~~~~~~~~~~~~~~~~~

**Symptom:** Flashing starts but fails partway through

**Solutions:**

1. **Put ESP32 in download mode manually:**

   - Hold BOOT button
   - Press RESET button
   - Release RESET
   - Release BOOT

2. **Check serial connection:**

   - Try different USB cable
   - Connect directly (avoid USB hubs)

3. **Verify firmware build:**

   .. code-block:: bash

      idf.py build
      ls -lh build/*.bin build/bootloader/*.bin build/partition_table/*.bin

Firefox/Safari Not Working
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Symptom:** Web flasher doesn't load or connect button missing

**Cause:** Web Serial API not supported in Firefox/Safari

**Solution:** Use Chrome, Edge, or Opera browser

References
----------

- `ESP Web Tools Documentation <https://esphome.github.io/esp-web-tools/>`_
- `Web Serial API Specification <https://wicg.github.io/serial/>`_
- `ESP-IDF Flash Partition Tables <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html>`_

See Also
--------

- :doc:`qemu-emulator` - Testing without hardware using QEMU
- :doc:`debugging` - GDB debugging workflows
