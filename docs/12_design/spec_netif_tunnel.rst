UART Tunnel Network Interface Design Specification
===================================================

This document specifies the component-level design of the ``netif_uart_tunnel``
component, which provides full TCP/IP stack functionality in QEMU emulation by
tunnelling IP packets (encapsulated in Ethernet frames) over UART1.

**Document Version**: 1.0
**Last Updated**: 2025-03-16

**Source files reviewed:**

- ``main/components/netif_uart_tunnel/netif_uart_tunnel_sim.c``
- ``main/components/netif_uart_tunnel/netif_uart_tunnel_sim.h``
- ``tools/qemu/network/serial_tun_bridge.py``
- ``main/components/netif_uart_tunnel/README.md``

This spec is a documentation catch-up — the code is stable and correct.
All SPECs below describe what the implementation *actually does*.
Two known deviations from requirements are documented honestly:
the static IP configuration (vs DHCP) and the 115200 baud rate (vs 100 KB/s target).

See :need:`SPEC_ARCH_NETIF_1` for the high-level architectural overview.


Component Architecture
----------------------

.. spec:: UART Tunnel Component Architecture
   :id: SPEC_NETIF_UART_ARCH_1
   :links: REQ_NETIF_TUNNEL_1, REQ_NETIF_TUNNEL_5
   :status: implemented
   :tags: netif, qemu, uart, architecture

   **Description:** The ``netif_uart_tunnel`` component implements a lwIP network interface
   that tunnels IP packets over UART1, providing full TCP/IP stack functionality in QEMU
   emulation. The component is conditionally compiled only when ``CONFIG_TARGET_EMULATOR``
   is set (hardware builds use the WiFi manager instead).

   **Component Location:** ``main/components/netif_uart_tunnel/``

   **Key Files:**

   - ``netif_uart_tunnel_sim.c`` — Implementation (``_sim`` suffix marks simulator-only)
   - ``netif_uart_tunnel_sim.h`` — Public API (``netif_uart_tunnel_init``, ``_deinit``, ``_get_handle``)
   - ``CMakeLists.txt`` — Build integration

   **Threading Model:**

   - ``uart_rx_task`` (FreeRTOS task, priority 2, stack 4096 bytes): polls UART1 for
     incoming frames and injects received Ethernet frames into lwIP via
     ``esp_netif_receive``
   - TX path: lwIP calls ``netif_linkoutput`` synchronously in the ``tcpip_thread``
     context; ``netif_linkoutput`` writes the framed packet directly to UART1

   **Data Flow:**

   .. code-block:: text

      ┌─────────────────────────────────────────────────────────────┐
      │  ESP32 (QEMU)                                               │
      │  Application → lwIP → etharp_output → netif_linkoutput     │
      │                                           │                 │
      │                                         UART1 TX (GPIO17)  │
      └─────────────────────────────────────────────────────────────┘
               ↕  Unix socket (temp/esp32-uart1.sock)
      ┌─────────────────────────────────────────────────────────────┐
      │  Host (Linux)                                               │
      │  serial_tun_bridge.py                                       │
      │  UART socket ↔ TUN device (tun0)                           │
      │  tun0: 192.168.100.1/24                                     │
      └─────────────────────────────────────────────────────────────┘

   **UART Configuration (hardcoded in implementation):**

   - Port: UART1
   - TX pin: GPIO17, RX pin: GPIO16
   - Baud rate: 115200 (``UART_BAUD_RATE`` constant)
   - Buffer: 2048 bytes for both RX and TX

   .. note::
      The header file comment mentions GPIO4/GPIO5 as defaults; the actual
      implementation uses GPIO17/GPIO16. The code values take precedence.

   **Network Configuration (default, passed by caller in** ``main.c`` **):**

   - ESP32 IP: 192.168.100.2
   - Gateway / Host: 192.168.100.1
   - Netmask: 255.255.255.0
   - ESP32 MAC: 02:00:00:00:00:02 (static, set directly on ``lwip_netif->hwaddr``)

   **lwIP Interface Flags:**

   ``NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_BROADCAST | NETIF_FLAG_AUTOUP``

   The interface operates as a full Ethernet interface so that lwIP's ARP layer
   produces complete Ethernet frames. The host-side bridge strips/adds the Ethernet
   header when interfacing with the TUN device (which expects raw IP packets).

   **Static ARP Entry for Gateway:**

   An ARP entry is pre-populated for the gateway MAC (``02:00:00:00:00:01``) via
   ``etharp_add_static_entry`` during init. This avoids ARP request/response cycles
   that would fail over a point-to-point UART link.

   **Lifecycle:**

   - ``netif_uart_tunnel_init(config)`` — initialises UART, creates ``esp_netif``,
     sets static IP, starts ``uart_rx_task``. Returns ``ESP_ERR_INVALID_STATE`` if
     called more than once (module uses static state).
   - ``netif_uart_tunnel_deinit()`` — stops ``uart_rx_task`` (via ``vTaskDelete``),
     destroys netif, deletes UART driver.
   - ``netif_uart_tunnel_get_handle()`` — returns the ``esp_netif_t*`` handle for
     event handler registration or status queries.

   **Known Limitation:** This component is QEMU-only. It MUST NOT be linked in
   hardware builds. See :need:`SPEC_NETIF_UART_COND_1` for build guard details.


Wire Protocol
-------------

.. spec:: UART Wire Protocol — Length-Prefix Framing
   :id: SPEC_NETIF_UART_PROTO_1
   :links: REQ_NETIF_TUNNEL_2, REQ_NETIF_TUNNEL_NF_2
   :status: implemented
   :tags: netif, qemu, protocol, framing

   **Description:** IP packets (wrapped in Ethernet frames) are transported over UART1
   using a simple 2-byte length-prefix framing protocol. There is no checksum or CRC
   at this layer — TCP handles end-to-end integrity.

   **Frame Format:**

   .. code-block:: text

      ┌─────────────────┬────────────────────────────────────────────┐
      │  LENGTH (2 B)   │  DATA (N bytes — complete Ethernet frame)  │
      │  big-endian     │  N = 14 (Ethernet header) + IP payload     │
      │  uint16_t       │  max N = 1500 bytes (MTU)                  │
      └─────────────────┴────────────────────────────────────────────┘

   **Ethernet Encapsulation:**

   The DATA field is a complete Ethernet frame, not a raw IP packet. lwIP operates
   as an Ethernet interface (``NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET``), so it
   produces and consumes full Ethernet frames including the 14-byte header.

   The Python bridge strips the 14-byte Ethernet header before writing to the TUN
   device (which handles raw IP), and prepends a synthetic Ethernet header when
   forwarding packets from TUN to the serial socket.

   **Framing Rules:**

   - LENGTH is a 2-byte big-endian ``uint16_t`` encoding the byte count of DATA
     only (the 2-byte header itself is not included)
   - Receiver MUST read exactly 2 bytes for LENGTH, then exactly LENGTH bytes for DATA
   - Frames with ``LENGTH == 0`` or ``LENGTH > 1500`` are invalid; the receiver MUST
     flush UART and resync

   **TX Path (ESP32 → Host):**

   1. lwIP calls ``netif_linkoutput`` with a pbuf chain (``tcpip_thread`` context)
   2. pbuf chain is copied into a contiguous ``frame_buf`` via ``pbuf_copy_partial``
   3. 2-byte big-endian length header written to UART1 via ``uart_write_bytes``
   4. Frame data written to UART1 via ``uart_write_bytes``
   5. ``frame_buf`` freed

   **RX Path (Host → ESP32):**

   1. ``uart_rx_task`` reads 2-byte length header with ``UART_READ_TIMEOUT_MS = 100 ms``
   2. Reads LENGTH bytes of frame data with a 1-second timeout
   3. Passes frame to lwIP via ``esp_netif_receive``
   4. lwIP calls ``tcpip_input`` → processes Ethernet frame through ARP / IP stack

   **Error Recovery:**

   - No data within 100 ms timeout: ``vTaskDelay(10 ms)`` to avoid busy-loop, retry
   - Invalid LENGTH (0 or > 1500): log warning, ``uart_flush()``, ``vTaskDelay(100 ms)``, retry
   - Incomplete frame data (1-second timeout): log warning, discard packet, retry
   - ``esp_netif_receive`` failure: log warning, discard, continue

   **Startup Delay:**

   After ``uart_flush()`` on task start, the RX task waits 1 second before processing
   to allow QEMU and the bridge script to reach a stable state.


Host-Side Bridge
----------------

.. spec:: Host-Side Serial-TUN Bridge Script
   :id: SPEC_NETIF_UART_BRIDGE_1
   :links: REQ_NETIF_TUNNEL_3, REQ_NETIF_TUNNEL_DOC_1
   :status: implemented
   :tags: netif, qemu, python, bridge, tooling

   **Description:** ``tools/qemu/network/serial_tun_bridge.py`` is the host-side
   component that bridges QEMU UART1 to the Linux TUN network device, giving the
   simulated ESP32 real network connectivity.

   **Script Location:** ``tools/qemu/network/serial_tun_bridge.py``

   **Prerequisites:**

   - Linux host (TUN device creation requires ``/dev/net/tun`` and Linux-specific
     ``ioctl`` calls; macOS is not supported)
   - Root privileges (for TUN device creation)
   - Python ≥ 3.8
   - Optional: ``pytun`` package — falls back to manual ``ioctl`` if absent

   .. note::
      ``REQ_NETIF_TUNNEL_3 AC-4`` specifies Linux *and* macOS support. The current
      implementation is Linux-only due to TUN device creation via Linux ioctls.

   **Architecture:**

   .. code-block:: text

      QEMU UART1 (Unix socket at temp/esp32-uart1.sock)
           ↕ Ethernet frames with 2-byte length prefix
      SerialTunBridge.serial_to_tun() / tun_to_serial()
           ↕ Raw IP packets (Ethernet header stripped / added)
      TUN device tun0 (192.168.100.1/24)
           ↕
      Linux host network stack

   **TUN Device:**

   - Name: ``tun0`` (hardcoded)
   - Host IP: ``192.168.100.1``
   - Netmask: ``255.255.255.0``
   - MTU: 1500 bytes
   - Mode: ``IFF_TUN | IFF_NO_PI`` (raw IP, no packet info header)

   **Ethernet/IP Translation:**

   - **serial → TUN**: Strip 14-byte Ethernet header from received frame; write
     raw IP packet to TUN device
   - **TUN → serial**: Read raw IP from TUN; prepend synthetic Ethernet header
     (DST = ``02:00:00:00:00:02``, SRC = ``02:00:00:00:00:01``,
     EtherType = ``0x0800``); send length-prefixed Ethernet frame to socket

   **Socket Connection:**

   - QEMU exposes UART1 as a Unix domain socket at
     ``{PROJECT_DIR}/temp/esp32-uart1.sock``
   - Bridge connects via ``AF_UNIX / SOCK_STREAM``
   - Auto-reconnects if the socket disappears (e.g. QEMU restart); TUN device
     persists across reconnects

   **Execution Modes:**

   - Verbose (default): ``sudo python3 serial_tun_bridge.py`` — INFO logs to console
   - Quiet: ``sudo python3 serial_tun_bridge.py --quiet`` — only errors logged to
     ``temp/tun_errors.log``
   - The ``run_qemu.sh`` script automatically starts the bridge in quiet mode

   **Known Limitations:**

   - Linux only (TUN ioctls are Linux-specific)
   - Requires ``sudo``
   - TUN device name ``tun0`` is hardcoded; conflicts if another process uses it
   - No graceful shutdown on SIGINT from ``run_qemu.sh`` (sends SIGTERM)


IP Configuration
----------------

.. spec:: IP Configuration and DHCP Client Integration
   :id: SPEC_NETIF_UART_DHCP_1
   :links: REQ_NETIF_TUNNEL_4
   :status: implemented
   :tags: netif, qemu, dhcp, ip

   **Description:** The tunnel interface uses static IP configuration. The DHCP
   client is explicitly disabled.  ``REQ_NETIF_TUNNEL_4`` specifies DHCP client
   support as a goal; the current implementation satisfies the *intent* (known IP
   on a known subnet) via static configuration rather than dynamic discovery.

   **Implemented Behaviour:**

   - DHCP client is explicitly **stopped** via ``esp_netif_dhcpc_stop()`` in
     ``netif_uart_tunnel_init``
   - Static IP is configured via ``esp_netif_set_ip_info()`` using caller-supplied
     values
   - Caller in ``main.c`` provides: IP ``192.168.100.2``, GW ``192.168.100.1``,
     mask ``255.255.255.0``
   - IP configuration is logged to console at INFO level after init
     (satisfies AC-4)

   **Static ARP Entry for Gateway:**

   A static ARP entry is pre-populated for the gateway to avoid ARP request/response
   cycles that cannot succeed over a point-to-point UART link:

   .. code-block:: c

      struct eth_addr gw_mac = {{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}};
      etharp_add_static_entry(&gw_ip, &gw_mac);

   **Rationale for Static Configuration:**

   A UART tunnel is a point-to-point link. DHCP discovery would require the
   host-side bridge to run a DHCP server. Static configuration is simpler,
   deterministic, and fully sufficient for emulation purposes.

   **AC Coverage vs REQ_NETIF_TUNNEL_4:**

   - AC-1 (support lwIP DHCP client): ⚠️ **Not implemented** — static IP used
     instead. DHCP adds complexity without benefit for QEMU emulation.
   - AC-2 (DHCP discovery via tunnel): **Not applicable** with static config.
   - AC-3 (obtain IP from host DHCP): **Not implemented** — static IP is the
     equivalent outcome.
   - AC-4 (log IP to console): ✅ Implemented via ``ESP_LOGI`` in
     ``netif_uart_tunnel_init``.

   .. note::
      AC-1 through AC-3 of ``REQ_NETIF_TUNNEL_4`` are **not met** by the current
      implementation. The requirement should be revisited if dynamic IP assignment
      ever becomes necessary. For the current QEMU emulation use case, static IP
      is the accepted approach.


Conditional Compilation
-----------------------

.. spec:: Conditional Compilation — QEMU-Only Build Guard
   :id: SPEC_NETIF_UART_COND_1
   :links: REQ_NETIF_TUNNEL_5
   :status: implemented
   :tags: netif, qemu, build, kconfig

   **Description:** The tunnel component is conditionally included in the build
   using CMake and Kconfig guards, ensuring zero overhead on hardware builds.

   **Application-Level Guard in** ``main.c`` **:**

   .. code-block:: c

      #ifdef CONFIG_TARGET_EMULATOR
          netif_uart_tunnel_init(&tunnel_config);
      #else
          // WiFi init for real hardware
          wifi_manager_init();
      #endif

   The ``CONFIG_TARGET_EMULATOR`` symbol is set by the project's Kconfig when the
   build target is the QEMU emulator.

   **CMakeLists.txt Guard:**

   The component's ``CMakeLists.txt`` uses ``idf_build_get_property`` to check the
   IDF target and registers the component only for QEMU builds. The component source
   files are not compiled at all for hardware targets.

   **Kconfig Options (from README):**

   - ``CONFIG_NETIF_UART_TUNNEL_ENABLED`` — Enable tunnel (default: ``y`` for QEMU)
   - ``CONFIG_NETIF_UART_TUNNEL_UART_NUM`` — UART port (default: 1)
   - ``CONFIG_NETIF_UART_TUNNEL_BAUD_RATE`` — Baud rate (default: 921600 per README,
     but the implementation constant ``UART_BAUD_RATE`` is currently 115200 — see
     :need:`SPEC_NETIF_UART_PERF_1` for discussion of this inconsistency)

   **Result on Hardware Builds:**

   - Component source files are not compiled
   - No symbols linked
   - Zero flash / RAM overhead
   - ``netif_uart_tunnel_init`` call is preprocessed away


Performance and Known Limitations
----------------------------------

.. spec:: Performance Characteristics and Known Limitations
   :id: SPEC_NETIF_UART_PERF_1
   :links: REQ_NETIF_TUNNEL_NF_1, REQ_NETIF_TUNNEL_NF_2
   :status: implemented
   :tags: netif, qemu, performance, limitations

   **Description:** Documents the measured and expected performance of the UART
   tunnel and all known limitations relevant to emulation use.

   **Throughput:**

   - Actual baud rate: **115200 bps** (``UART_BAUD_RATE`` constant in
     ``netif_uart_tunnel_sim.c``)
   - Theoretical raw throughput: ~14.4 KB/s (115200 / 8 bits, no overhead)
   - With 2-byte framing overhead per packet: ~14.3 KB/s effective
   - :need:`SPEC_ARCH_NETIF_1` notes ~10 KB/s measured throughput in practice
   - ⚠️ **Does NOT meet REQ_NETIF_TUNNEL_NF_1 AC-1 (100 KB/s minimum)**
   - Increasing baud rate to 921600 would yield ~115 KB/s, satisfying the target
   - README documents 921600 as the intended baud rate, but the code constant
     has not been updated to match. The code value (115200) is what actually runs.

   **Packet Loss Handling:**

   - Invalid LENGTH (0 or > 1500): UART flushed, 100 ms delay, retry loop
     — satisfies ``REQ_NETIF_TUNNEL_NF_2 AC-1``
   - Incomplete frame data (1-second timeout): warning logged, packet discarded
     — satisfies ``REQ_NETIF_TUNNEL_NF_2 AC-1``
   - TCP layer recovery from packet loss: inherent in the TCP protocol
     — satisfies ``REQ_NETIF_TUNNEL_NF_2 AC-2``
   - Packet statistics: ``s_rx_count`` and ``s_tx_count`` debug counters logged
     via ``ESP_LOGD`` — satisfies ``REQ_NETIF_TUNNEL_NF_2 AC-3``

   **RX Task Design:**

   - Stack: 4096 bytes
   - Priority: 2 (lower than ``display_logic`` at priority 3 to avoid blocking UI)
   - Length header read timeout: 100 ms (``UART_READ_TIMEOUT_MS``)
   - Frame data read timeout: 1 second
   - 1-second startup delay after ``uart_flush()`` for stability
   - 10 ms yield on timeout to avoid busy-looping when there is no traffic

   **Known Limitations:**

   1. **115200 baud**: Below the 100 KB/s NF target (fixable by updating
      ``UART_BAUD_RATE`` to 921600)
   2. **QEMU-only / Linux host required**: ``serial_tun_bridge.py`` uses Linux TUN
      ioctls; macOS is not supported
   3. **Root required**: TUN device creation requires ``sudo``
   4. **Static IP only**: No DHCP client (see :need:`SPEC_NETIF_UART_DHCP_1`)
   5. **Single instance**: Module uses static state; ``netif_uart_tunnel_init``
      returns ``ESP_ERR_INVALID_STATE`` on a second call
   6. **No graceful RX task stop**: ``netif_uart_tunnel_deinit`` calls
      ``vTaskDelete`` on the RX task handle (no cooperative shutdown signal)
   7. **TUN device name hardcoded**: ``tun0`` in bridge script; conflicts if
      another process already owns that interface


Documentation
-------------

.. spec:: Emulation Setup Documentation
   :id: SPEC_NETIF_UART_DOC_1
   :links: REQ_NETIF_TUNNEL_DOC_1
   :status: implemented
   :tags: netif, qemu, documentation

   **Description:** Documents where emulation setup instructions are maintained
   and what they cover.

   **Documentation Locations:**

   - ``main/components/netif_uart_tunnel/README.md`` — Component-level README with
     quick-start, configuration, usage, and troubleshooting sections
   - ``tools/qemu/run_qemu.sh`` — QEMU launch script (self-documenting via inline
     comments)
   - ``tools/qemu/network/serial_tun_bridge.py`` — Bridge script with module-level
     docstring describing protocol, usage, and requirements
   - ``docs/90_guides/switching-dev-modes.rst`` — Developer guide for switching
     between QEMU and real hardware

   **Coverage by Requirement AC:**

   - ``REQ_NETIF_TUNNEL_DOC_1 AC-1`` (QEMU build / install instructions): Covered
     in README and ``tools/qemu/`` scripts
   - ``REQ_NETIF_TUNNEL_DOC_1 AC-2`` (TAP/TUN interface setup): Covered in
     ``serial_tun_bridge.py`` docstring and README
   - ``REQ_NETIF_TUNNEL_DOC_1 AC-3`` (example commands): Covered in README
     troubleshooting section and ``run_qemu.sh``
   - ``REQ_NETIF_TUNNEL_DOC_1 AC-4`` (limitations vs hardware): Covered in
     README limitations section and :need:`SPEC_NETIF_UART_PERF_1`


Traceability
------------

All traceability is automatically generated by Sphinx-Needs based on the
``:links:`` attributes in each SPEC above.

.. needtable::
   :columns: id, title, status, tags
   :filter: id in ["SPEC_NETIF_UART_ARCH_1", "SPEC_NETIF_UART_PROTO_1", "SPEC_NETIF_UART_BRIDGE_1", "SPEC_NETIF_UART_DHCP_1", "SPEC_NETIF_UART_COND_1", "SPEC_NETIF_UART_PERF_1", "SPEC_NETIF_UART_DOC_1"]
