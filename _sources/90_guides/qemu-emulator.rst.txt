QEMU Emulator Guide
===================

The ESP32 Distance Sensor project includes full QEMU emulation support, allowing you to develop and test without physical hardware. The emulator includes complete network functionality via a UART-based IP tunnel.

Quick Start
-----------

Starting QEMU
~~~~~~~~~~~~~

**For Normal Operation:**

.. code-block:: bash

   # From project root - runs immediately
   ./tools/qemu/run_qemu.sh

**For Debugging:**

.. code-block:: bash

   # Starts QEMU and waits for GDB debugger
   ./tools/qemu/run_qemu_debug.sh

Or use VS Code tasks:

- **Run QEMU (No Debug)** - Start emulator immediately
- **Run QEMU Debug** - Start with GDB support, then press F5 to attach debugger

Both scripts automatically:

- ✅ Build the project if needed (incremental, fast)
- ✅ Start the network stack (TUN device, bridge)
- ✅ Create Unix domain sockets for UART communication
- ✅ Launch QEMU with proper configuration

Stopping QEMU
~~~~~~~~~~~~~

.. code-block:: bash

   ./tools/qemu/stop_qemu.sh

Or use VS Code task: **Stop QEMU**

Accessing the Web Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once QEMU is running:

.. code-block:: bash

   # Direct access to emulated ESP32
   curl http://192.168.100.2/

   # Via HTTP proxy (if configured)
   curl http://localhost:8080/

The web interface should be accessible at ``http://192.168.100.2`` in your browser.

Architecture Overview
---------------------

.. code-block:: text

   ┌────────────────────────────────────────────────────────────┐
   │                    Host System (Linux)                     │
   │                                                            │
   │  ┌──────────┐           ┌──────────────────┐              │
   │  │ Browser  │──────────▶│   tun0 Device    │              │
   │  │          │    HTTP   │  192.168.100.1   │              │
   │  └──────────┘           └──────────────────┘              │
   │                                  │                         │
   │                         ┌────────▼─────────┐               │
   │                         │   TUN Bridge     │               │
   │                         │   (Python)       │               │
   │                         └────────┬─────────┘               │
   │                                  │ Unix Socket             │
   │                         temp/esp32-uart1.sock              │
   └──────────────────────────────────┼────────────────────────┘
                                      │
                                      │ QEMU chardev
   ┌──────────────────────────────────┼────────────────────────┐
   │                   ESP32 QEMU     │                        │
   │                                  ▼                        │
   │  ┌────────────┐    ┌──────────┐    ┌──────────────┐      │
   │  │    Web     │    │   lwIP   │    │    UART1     │      │
   │  │   Server   │◄──►│  Stack   │◄──►│   Driver     │      │
   │  │   :80      │    │ 192.168. │    │              │      │
   │  │            │    │ 100.2/24 │    └──────────────┘      │
   │  └────────────┘    └──────────┘                          │
   │                                                           │
   │  Console Logs (ESP_LOG*)                                 │
   │         │                                                 │
   │         ▼                                                 │
   │  ┌──────────────┐      Unix Socket                       │
   │  │    UART0     │──────temp/esp32-uart0.sock────────────▶│
   │  └──────────────┘                                        │
   └───────────────────────────────────────────────────────────┘

Network Configuration
---------------------

IP Addresses
~~~~~~~~~~~~

+---------------------+-------------------+----------------------------+
| Component           | IP Address        | Description                |
+=====================+===================+============================+
| Host TUN device     | 192.168.100.1/24  | Gateway for emulated ESP32 |
+---------------------+-------------------+----------------------------+
| ESP32 QEMU          | 192.168.100.2/24  | Emulated device IP         |
+---------------------+-------------------+----------------------------+

Communication Channels
~~~~~~~~~~~~~~~~~~~~~~

+----------------------+-----------------------------+--------------------------------+
| Channel              | Location                    | Purpose                        |
+======================+=============================+================================+
| UART0 Socket         | ``temp/esp32-uart0.sock``   | Console output (ESP_LOG*)      |
+----------------------+-----------------------------+--------------------------------+
| UART1 Socket         | ``temp/esp32-uart1.sock``   | Network tunnel (IP packets)    |
+----------------------+-----------------------------+--------------------------------+
| GDB Port (debug)     | TCP 3333                    | Debugger connection (when -d)  |
+----------------------+-----------------------------+--------------------------------+

.. note::
   This project uses **Unix domain sockets** for UART communication instead of TCP ports. This eliminates "port already in use" errors and provides better performance.

How It Works
------------

UART-Based IP Tunnel
~~~~~~~~~~~~~~~~~~~~~

The emulator uses UART1 as a network interface:

1. **Ethernet Frame Encapsulation**: IP packets are wrapped in Ethernet frames
2. **Length Prefix Protocol**: Each frame is prefixed with 2-byte length (big-endian)
3. **UART Transport**: Frames are transmitted over UART1 (115200 baud)
4. **TUN Bridge**: Python script bridges UART ↔ TUN device

Network Stack
~~~~~~~~~~~~~

.. code-block:: text

   Application (Web Server, your code)
            ↓
       lwIP TCP/IP Stack
            ↓
   Custom UART Network Interface (netif_uart_tunnel_sim.c)
            ↓
         UART1 Driver
            ↓
      QEMU Serial Device
            ↓
       TUN Bridge (Python)
            ↓
         Linux TUN Device
            ↓
        Host Network Stack

Frame Format
~~~~~~~~~~~~

.. code-block:: text

   ┌───────────────┬──────────────────────────────────────────┐
   │ Frame Length  │         Ethernet Frame                   │
   │   (2 bytes)   │    (14-byte header + IP packet)          │
   │  Big Endian   │                                          │
   ├───────────────┼──────────────────────────────────────────┤
   │  [HI] [LO]    │ [DST_MAC:6][SRC_MAC:6][TYPE:2][IP DATA]  │
   └───────────────┴──────────────────────────────────────────┘

**Example:** ICMP Echo Request (98 bytes)

.. code-block:: text

   Length: 0x00 0x62 (98 bytes)
   Ethernet:
        Dst MAC: 02:00:00:00:00:02 (ESP32)
        Src MAC: 02:00:00:00:00:01 (Host)
        Type:    0x08 0x00 (IPv4)
   IP Packet:
        Src IP: 192.168.100.1
        Dst IP: 192.168.100.2
        Protocol: ICMP

Testing Network Connectivity
-----------------------------

Ping Test
~~~~~~~~~

.. code-block:: bash

   # Ping the emulated ESP32
   ping -c 4 192.168.100.2

Expected output:

.. code-block:: text

   64 bytes from 192.168.100.2: icmp_seq=1 ttl=64 time=5.2 ms
   64 bytes from 192.168.100.2: icmp_seq=2 ttl=64 time=3.8 ms

HTTP Test
~~~~~~~~~

.. code-block:: bash

   # Direct access
   curl http://192.168.100.2/

   # Via proxy
   curl http://localhost:8080/

Monitor Network Traffic
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Watch TUN device traffic
   sudo tcpdump -i tun0 -n

   # Monitor UART traffic in QEMU logs
   # Look for "RX:" and "TX:" messages

Troubleshooting
---------------

QEMU Won't Start
~~~~~~~~~~~~~~~~

**Problem:** Script fails to start QEMU

**Solutions:**

.. code-block:: bash

   # Check if QEMU is already running
   ps aux | grep qemu-system-xtensa

   # Stop any existing QEMU instances
   ./tools/qemu/stop_qemu.sh

   # Clean old socket files
   rm -f temp/esp32-uart*.sock

   # Rebuild and try again
   idf.py build
   ./tools/qemu/run_qemu.sh

Network Not Working
~~~~~~~~~~~~~~~~~~~

**Problem:** Can't ping or access web server

**Checks:**

.. code-block:: bash

   # 1. Verify TUN device exists
   ip addr show tun0

   # 2. Check TUN bridge is running
   ps aux | grep serial_tun_bridge

   # 3. Verify UART1 socket exists
   ls -l temp/esp32-uart1.sock

   # 4. Restart everything
   ./tools/qemu/stop_qemu.sh
   ./tools/qemu/run_qemu.sh

No Console Output
~~~~~~~~~~~~~~~~~

**Problem:** No logs from QEMU

**Solution:**

.. code-block:: bash

   # Use the UART0 terminal viewer
   ./tools/qemu/uart0_terminal.sh

This connects to the UART0 Unix socket and displays ESP_LOG* output.

Debugging with GDB
~~~~~~~~~~~~~~~~~~

**Problem:** Want to debug but QEMU runs too fast

**Solution:**

Use the debug script and VS Code integration:

1. Start QEMU in debug mode: ``./tools/qemu/run_qemu_debug.sh`` (or use VS Code task)
2. QEMU will wait for debugger connection
3. Press **F5** in VS Code to attach debugger
4. Set breakpoints and step through code

See :doc:`debugging` for complete GDB debugging workflow.

Advanced Usage
--------------

Debugging Network Issues
~~~~~~~~~~~~~~~~~~~~~~~~

Enable verbose logging in ``netif_uart_tunnel_sim.c``:

.. code-block:: c

   // Temporarily change log level
   #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

Then rebuild and watch detailed packet flow:

.. code-block:: bash

   idf.py build
   ./tools/run-qemu-network.sh

Custom Network Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Edit ``main/components/netif_uart_tunnel/netif_uart_tunnel_sim.c``:

.. code-block:: c

   // Change IP address
   #define ESP32_IP "192.168.100.2"
   #define GATEWAY_IP "192.168.100.1"
   #define NETMASK "255.255.255.0"

Running QEMU
~~~~~~~~~~~~

**Two modes available:**

1. **Normal Mode** (runs immediately):

   .. code-block:: bash

      ./tools/qemu/run_qemu.sh

2. **Debug Mode** (waits for GDB):

   .. code-block:: bash

      ./tools/qemu/run_qemu_debug.sh

Both modes use the same QEMU instance with Unix socket communication. Debug mode adds the ``-d`` flag to enable GDB support and wait for debugger attachment.

Technical Deep Dive
-------------------

For detailed information about the network implementation, packet flow, and lwIP integration, see :doc:`qemu-network-internals`.

Known Limitations
-----------------

- **UART Speed**: Limited to 115200 baud (adequate for HTTP, slow for large transfers)
- **No WiFi Simulation**: Uses direct IP connectivity instead of WiFi AP/STA modes
- **Browser Caching**: Web interface may cache old versions (use Ctrl+F5 to refresh)
- **QEMU Performance**: Slower than real hardware, but sufficient for testing
- **HTTPS Not Working**: SSL/TLS support in QEMU is work in progress

Next Steps
----------

- :doc:`debugging` - Set breakpoints and step through code
- :doc:`qemu-network-internals` - Understand packet flow in detail
- :doc:`devcontainer` - Configure your development environment
