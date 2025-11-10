Network Interface Tunnel Requirements
======================================

This document specifies component-level requirements for the UART-based network tunnel driver, enabling QEMU emulation with external network connectivity.

Component Overview
------------------

The network interface tunnel component provides a bridge between QEMU's simulated UART and the host's network stack, enabling network-dependent features to be tested in emulation. These requirements refine the high-level system requirement :need:`REQ_SYS_SIM_1`.


Functional Requirements
-----------------------

.. req:: QEMU UART Network Bridge
   :id: REQ_NETIF_TUNNEL_1
   :links: REQ_SYS_SIM_1
   :status: approved
   :priority: mandatory
   :tags: emulation, network, qemu, uart

   **Description:** The system SHALL provide a custom network interface driver that bridges ESP32's UART to the host network stack when running in QEMU emulation.

   **Rationale:** QEMU does not natively support ESP32 WiFi emulation. A UART tunnel enables network connectivity for testing web server and WiFi-dependent features.

   **Acceptance Criteria:**

   - AC-1: Driver SHALL register as lwIP network interface
   - AC-2: Driver SHALL use UART1 for packet transport to/from QEMU
   - AC-3: Driver SHALL be conditionally compiled only for QEMU target
   - AC-4: Driver SHALL not interfere with real hardware WiFi driver


.. req:: Packet Encapsulation
   :id: REQ_NETIF_TUNNEL_2
   :links: REQ_NETIF_TUNNEL_1
   :status: approved
   :priority: mandatory
   :tags: emulation, protocol

   **Description:** The tunnel driver SHALL encapsulate Ethernet frames for transmission over UART with appropriate framing and error detection.

   **Rationale:** UART is a byte-stream transport; Ethernet frame boundaries must be preserved.

   **Acceptance Criteria:**

   - AC-1: Driver SHALL frame packets with length prefix or delimiter
   - AC-2: Driver SHALL detect and discard corrupted packets
   - AC-3: Framing SHALL support MTU-sized packets (1500 bytes)
   - AC-4: Framing overhead SHALL be minimal (<10 bytes per packet)


.. req:: Host-Side Bridge Script
   :id: REQ_NETIF_TUNNEL_3
   :links: REQ_NETIF_TUNNEL_1
   :status: approved
   :priority: mandatory
   :tags: emulation, tooling, host

   **Description:** The project SHALL provide a Python script that bridges QEMU's UART to the host's TAP network interface.

   **Rationale:** QEMU UART output must be forwarded to host network stack for external connectivity.

   **Acceptance Criteria:**

   - AC-1: Script SHALL create TAP interface on host OS
   - AC-2: Script SHALL forward packets bidirectionally between UART and TAP
   - AC-3: Script SHALL handle QEMU's character device socket protocol
   - AC-4: Script SHALL be executable on Linux and macOS
   - AC-5: Script SHALL provide logging for debugging


.. req:: DHCP Client Support
   :id: REQ_NETIF_TUNNEL_4
   :links: REQ_NETIF_TUNNEL_1, REQ_SYS_NET_1
   :status: approved
   :priority: mandatory
   :tags: emulation, network, dhcp

   **Description:** The tunnel network interface SHALL support DHCP client operation to obtain IP address from host network.

   **Rationale:** Automatic IP configuration simplifies emulation setup and mirrors real WiFi behavior.

   **Acceptance Criteria:**

   - AC-1: Driver SHALL support lwIP DHCP client
   - AC-2: DHCP discovery packets SHALL be forwarded via UART tunnel
   - AC-3: Driver SHALL obtain valid IP address from host DHCP server
   - AC-4: IP configuration SHALL be logged to console


.. req:: Conditional Compilation
   :id: REQ_NETIF_TUNNEL_5
   :links: REQ_NETIF_TUNNEL_1
   :status: approved
   :priority: mandatory
   :tags: emulation, build

   **Description:** The tunnel driver SHALL be conditionally compiled only when building for QEMU target, with zero overhead on real hardware builds.

   **Rationale:** Real hardware uses WiFi driver; tunnel driver is emulation-specific.

   **Acceptance Criteria:**

   - AC-1: Driver SHALL be wrapped in `#ifdef CONFIG_IDF_TARGET_ESP32_QEMU`
   - AC-2: Driver SHALL NOT be linked in hardware builds
   - AC-3: Application code SHALL detect emulation mode at runtime
   - AC-4: Build system SHALL provide clear documentation for enabling QEMU mode


Performance Requirements
------------------------

.. req:: Tunnel Throughput
   :id: REQ_NETIF_TUNNEL_NF_1
   :links: REQ_NETIF_TUNNEL_1
   :status: approved
   :priority: optional
   :tags: emulation, performance

   **Description:** The UART tunnel SHOULD support sufficient throughput for web interface testing (minimum 100 KB/s).

   **Rationale:** Web page loads and AJAX updates require reasonable bandwidth.

   **Acceptance Criteria:**

   - AC-1: Tunnel SHALL sustain at least 100 KB/s bidirectional throughput
   - AC-2: Web page loads SHALL complete within 5 seconds over tunnel
   - AC-3: UART baud rate SHALL be configurable (default 921600)


.. req:: Packet Loss Handling
   :id: REQ_NETIF_TUNNEL_NF_2
   :links: REQ_NETIF_TUNNEL_2
   :status: approved
   :priority: optional
   :tags: emulation, reliability

   **Description:** The tunnel driver SHOULD handle UART packet loss gracefully without crashing.

   **Rationale:** QEMU's UART emulation may occasionally drop bytes under high load.

   **Acceptance Criteria:**

   - AC-1: Driver SHALL detect framing errors and discard partial packets
   - AC-2: TCP connections SHALL recover from occasional packet loss
   - AC-3: Driver SHALL log packet loss statistics for debugging


Documentation Requirements
---------------------------

.. req:: Emulation Setup Documentation
   :id: REQ_NETIF_TUNNEL_DOC_1
   :links: REQ_NETIF_TUNNEL_3, REQ_SYS_SIM_1
   :status: approved
   :priority: mandatory
   :tags: emulation, documentation

   **Description:** The project SHALL provide clear documentation for setting up and using QEMU emulation with network tunnel.

   **Rationale:** Developers need step-by-step instructions to use emulation features.

   **Acceptance Criteria:**

   - AC-1: Documentation SHALL include QEMU build/installation instructions
   - AC-2: Documentation SHALL explain TAP interface setup on Linux/macOS
   - AC-3: Documentation SHALL provide example commands for running emulation
   - AC-4: Documentation SHALL describe limitations compared to real hardware


Traceability
------------

All traceability is automatically generated by Sphinx-Needs based on the `:links:` attributes in each requirement.

.. needtable::
   :columns: id, title, status, tags

.. needflow:: REQ_NETIF_TUNNEL_1
