QEMU Network Internals
======================

This document provides detailed technical information about the UART-based IP tunnel implementation for ESP32 QEMU emulation.

.. note::
   This is advanced technical documentation. For usage instructions, see :doc:`qemu-emulator`.

Architecture Overview
---------------------

.. code-block:: text

   ┌─────────────────────────────────────────────────────────────────┐
   │                        HOST (Linux)                             │
   │                                                                 │
   │  ┌──────────┐      ┌─────────────┐      ┌─────────────────┐     │
   │  │  ping    │─────▶│  tun0       │◀────▶│  TUN Bridge     │     │
   │  │  curl    │      │ 192.168.    │      │  (Python)       │     │
   │  └──────────┘      │ 100.1/24    │      └─────────────────┘     │
   │                    └─────────────┘               │              │
   │                                                  │ TCP:5556     │
   └──────────────────────────────────────────────────┼──────────────┘
                                                      │
                                                      │ QEMU chardev
   ┌──────────────────────────────────────────────────┼──────────────┐
   │                     ESP32 QEMU                   │              │
   │                                                  ▼              │
   │  ┌────────────┐      ┌──────────────┐      ┌─────────────┐      │
   │  │  Web       │      │   lwIP       │      │   UART1     │      │
   │  │  Server    │◀───▶│   Stack      │◀───▶│   Driver    │      │
   │  │  HTTP      │      │ 192.168.     │      │             │      │
   │  │            │      │ 100.2/24     │      └─────────────┘      │
   │  └────────────┘      └──────────────┘                           │
   │                                                                 │
   └─────────────────────────────────────────────────────────────────┘

Frame Format
------------

Ethernet Frame Encapsulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   ┌───────────────┬────────────────────────────────────────────┐
   │ Frame Length  │           Ethernet Frame                   │
   │   (2 bytes)   │         (14-byte header + IP)              │
   │  Big Endian   │                                            │
   ├───────────────┼────────────────────────────────────────────┤
   │  [HI] [LO]    │ [DST_MAC:6][SRC_MAC:6][TYPE:2][IP PACKET]  │
   └───────────────┴────────────────────────────────────────────┘

Example ICMP Echo Request (98 bytes total):

.. code-block:: text

   Length: 0x00 0x62 (98 bytes)
   Ethernet:
     Dst MAC: 02:00:00:00:00:02 (ESP32)
     Src MAC: 02:00:00:00:00:01 (Host)
     Type:    0x08 0x00 (IPv4)
   IP:
     Version/IHL: 0x45 (IPv4, 20 byte header)
     Protocol: 0x01 (ICMP)
     Src IP: 192.168.100.1 (0xC0 0xA8 0x64 0x01)
     Dst IP: 192.168.100.2 (0xC0 0xA8 0x64 0x02)
   ICMP:
     Type: 0x08 (Echo Request)
     Code: 0x00

Initialization Sequence
-----------------------

System Startup
~~~~~~~~~~~~~~

1. **Initialize Network Stack** (``wifi_manager_sim.c``)

   - ``esp_netif_init()``
   - ``esp_event_loop_create_default()``

2. **Initialize UART Tunnel Driver** (``netif_uart_tunnel_sim.c``)

   a) **Hardware Setup:**

      - Configure UART1: 115200 baud, GPIO 17/16
      - ``uart_driver_install()`` with 2KB RX/TX buffers

   b) **Create esp_netif:**

      - ``ESP_NETIF_INHERENT_DEFAULT_ETH()`` config
      - ``esp_netif_new()``

   c) **Configure Static IP:**

      - IP: 192.168.100.2
      - Gateway: 192.168.100.1
      - Netmask: 255.255.255.0

   d) **Direct lwIP Integration:**

      - Get lwIP netif handle
      - Set MAC: 02:00:00:00:00:02
      - Set flags: ETHARP | ETHERNET | BROADCAST
      - Register linkoutput callback
      - Set as default netif
      - Add static ARP entry for gateway

3. **Start RX Task**

   - FreeRTOS task polls UART for incoming frames
   - Priority 5 (high priority for network responsiveness)

Packet Flow
-----------

Outgoing (TX): ESP32 → Host
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. **Application Layer**

   - Application calls lwIP functions (e.g., ``send()``, ``httpd_resp_send()``)

2. **lwIP TCP/IP Stack**

   - TCP/UDP processing
   - IP header generation
   - Routing decision (uses UART netif)

3. **Ethernet Layer**

   - lwIP calls ``linkoutput`` callback
   - Function: ``netif_output()`` in ``netif_uart_tunnel_sim.c``

4. **Frame Preparation**

   - Extract Ethernet header from pbuf (14 bytes)
   - Get payload length
   - Prepare 2-byte length prefix (big-endian)

5. **UART Transmission**

   - Write length prefix to UART1
   - Write Ethernet frame to UART1
   - QEMU chardev forwards to TCP socket

6. **TUN Bridge** (Python)

   - Receives frame from TCP socket
   - Writes frame to TUN device
   - Linux kernel processes as Ethernet frame

7. **Host Network Stack**

   - Routes to appropriate application (curl, browser, etc.)

Incoming (RX): Host → ESP32
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. **Host Application**

   - Application sends packet (e.g., HTTP request)

2. **Linux Network Stack**

   - Routes to TUN device (192.168.100.2)

3. **TUN Bridge** (Python)

   - Reads Ethernet frame from TUN device
   - Writes length-prefixed frame to TCP socket

4. **QEMU Chardev**

   - Forwards data to emulated UART1

5. **ESP32 UART Driver**

   - RX interrupt triggers
   - Data copied to FreeRTOS queue

6. **RX Task** (``uart_rx_task``)

   - Reads 2-byte length prefix
   - Reads Ethernet frame (up to 1518 bytes)
   - Validates frame length and format

7. **lwIP Input**

   - Allocates pbuf
   - Copies frame data to pbuf
   - Calls ``netif->input()`` → ``tcpip_input()``

8. **lwIP TCP/IP Stack**

   - Ethernet processing
   - IP routing
   - TCP/UDP handling

9. **Application Layer**

   - Application receives data via socket

Critical Implementation Details
-------------------------------

Buffer Management
~~~~~~~~~~~~~~~~~

.. code-block:: c

   // RX buffer allocation
   #define UART_RX_BUF_SIZE 2048
   #define MAX_ETH_FRAME_SIZE 1518
   
   uint8_t frame_buffer[MAX_ETH_FRAME_SIZE];

**Strategy:**

- Fixed-size buffer on stack for frame assembly
- pbuf allocation only after complete frame received
- Minimizes heap fragmentation

UART Configuration
~~~~~~~~~~~~~~~~~~

.. code-block:: c

   uart_config_t uart_config = {
       .baud_rate = 115200,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
   };

**Performance:**

- 115200 baud ≈ 14.4 KB/s theoretical max
- Practical throughput: ~10 KB/s (overhead + framing)
- Adequate for HTTP, slow for large transfers

Error Handling
~~~~~~~~~~~~~~

.. code-block:: c

   // Length validation
   if (frame_len < 14 || frame_len > MAX_ETH_FRAME_SIZE) {
       ESP_LOGW(TAG, "Invalid frame length: %d", frame_len);
       continue;  // Skip malformed frame
   }

**Robustness:**

- Frame length validation prevents buffer overflows
- Timeout on incomplete frames (1 second)
- Automatic recovery from malformed packets

MAC Address Assignment
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c

   static const uint8_t esp32_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
   static const uint8_t host_mac[6]  = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};

**Design:**

- Locally administered MAC addresses (bit 1 set)
- Static assignment for predictable ARP behavior
- No MAC learning required

Static ARP Entry
~~~~~~~~~~~~~~~~

.. code-block:: c

   // Add gateway ARP entry
   ip4_addr_t gw_addr;
   IP4_ADDR(&gw_addr, 192, 168, 100, 1);
   etharp_add_static_entry(&gw_addr, (struct eth_addr*)host_mac);

**Why Static ARP:**

- Prevents ARP requests over UART (reduces overhead)
- Immediate connectivity without ARP handshake
- Simplifies bridge implementation

Performance Characteristics
---------------------------

Throughput
~~~~~~~~~~

- **HTTP GET small file**: ~5-8 KB/s
- **HTTP GET 1MB file**: ~10 KB/s sustained
- **Ping latency**: 3-8 ms typical

Bottlenecks
~~~~~~~~~~~

1. **UART Bandwidth**: 115200 baud limit
2. **Frame Overhead**: 2-byte length + 14-byte Ethernet header per packet
3. **QEMU Emulation**: CPU overhead vs. real hardware

Optimization Opportunities
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Increase Baud Rate**: 230400 or 460800 (requires TUN bridge update)

**Jumbo Frames**: Support 9KB frames (requires lwIP MTU change)

**DMA**: Use UART DMA for reduced CPU overhead

**Zero-Copy**: Direct pbuf allocation in UART callback

Debugging Network Issues
------------------------

Enable Verbose Logging
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c

   // In netif_uart_tunnel_sim.c
   #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

Capture UART Traffic
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Monitor UART1 in separate terminal
   ./tools/view_uart1.sh

Analyze with tcpdump
~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Capture TUN device traffic
   sudo tcpdump -i tun0 -w capture.pcap
   
   # Analyze with Wireshark
   wireshark capture.pcap

Check Frame Alignment
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Look for length/frame mismatches in logs
   grep "RX:" /tmp/qemu_uart*.log | head -20

Known Issues and Limitations
-----------------------------

Current Limitations
~~~~~~~~~~~~~~~~~~~

- **UART Speed**: 115200 baud limits throughput to ~10 KB/s
- **No WiFi Emulation**: Direct IP connectivity, no AP/STA simulation
- **Single Interface**: Cannot emulate multiple network interfaces
- **No Packet Loss**: Reliable UART, no wireless error simulation

Future Improvements
~~~~~~~~~~~~~~~~~~~

- **Higher Baud Rate**: 460800+ for better throughput
- **WiFi Event Simulation**: Emulate connection/disconnection events
- **Packet Loss Simulation**: Inject errors for robustness testing
- **Multiple Interfaces**: Support AP + STA simultaneously

Source Code Reference
---------------------

Key Files
~~~~~~~~~

- ``main/components/netif_uart_tunnel/netif_uart_tunnel_sim.c`` - ESP32 driver
- ``main/components/web_server/wifi_manager_sim.c`` - Network initialization
- ``tools/serial_tun_bridge.py`` - Host-side TUN bridge
- ``tools/run-qemu-network.sh`` - Launch script

Further Reading
---------------

- `lwIP Documentation <https://www.nongnu.org/lwip/>`_
- `ESP-IDF esp_netif Guide <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_netif.html>`_
- `Linux TUN/TAP <https://www.kernel.org/doc/Documentation/networking/tuntap.txt>`_
- `QEMU Chardev <https://www.qemu.org/docs/master/system/device-emulation.html#character-device-backends>`_
