# Change Document: netif-tunnel-spec

**Status**: in-progress
**Branch**: feature/netif-tunnel-spec
**Created**: 2025-03-16
**Author**: syspilot Change Agent

---

## Summary

Documentation catch-up task: Create `docs/12_design/spec_netif_tunnel.rst` to fill the
MECE gap G-1 identified in design analysis. The `netif_uart_tunnel` component has been
running in production without any component-level design spec. The code is stable; this
change documents what is already implemented.

**Scope**: New design spec file + index update. No code changes. No new requirements.
No new user stories.

**Gap**: 8 requirements (`REQ_NETIF_TUNNEL_1` through `REQ_NETIF_TUNNEL_NF_2` and
`REQ_NETIF_TUNNEL_DOC_1`) are not covered by any SPEC. The only existing coverage is
`SPEC_ARCH_NETIF_1` (high-level component overview in `spec_high_level_architecture.rst`),
which is intentionally brief and does not constitute a component-level design spec.

---

## Level 0: User Stories

**Status**: ✅ completed

### Analysis

The `netif_uart_tunnel` component serves developers testing in QEMU emulation — not
end users. The three existing user stories (`US_SETUP_1`, `US_RELIABLE_1`,
`US_DISPLAY_1`) are all end-user facing (garage user / maker). None link to
`REQ_SYS_SIM_1` or `REQ_SYS_SIM_2`.

The system requirements `REQ_SYS_SIM_1` (Emulator Support) and `REQ_SYS_SIM_2`
(Emulator Network Connectivity) sit in `req_system.rst` without explicit US links —
they represent an implicit developer/contributor story.

### Impacted User Stories

| ID | Title | Impact | Notes |
|----|-------|--------|-------|
| *(none)* | — | none | This is a documentation task for a developer-facing component. No end-user story is impacted. |

### Decision: No new User Story needed

**Rationale:** The missing spec documents an internal developer tool (QEMU emulation
bridge). The existing implicit requirement chain
`REQ_SYS_SIM_1 → REQ_SYS_SIM_2 → REQ_NETIF_TUNNEL_*` is sufficient. Adding a new
User Story for "a developer wants QEMU emulation" would be valid but is out of scope
for this gap-filling task. The MECE analysis flagged missing SPEC coverage, not missing
User Story coverage.

**Horizontal Check (MECE):**
- ✅ No contradictions with existing User Stories
- ✅ No redundancies — existing US are end-user focused, this component is developer tooling
- ✅ Gap acknowledged and deliberately deferred: a `US_DEV_SIM` story could be added
  in a future change if the project adopts developer personas in US scope

---

## Level 1: Requirements

**Status**: ✅ completed

### Impacted Requirements (existing, all approved — no changes needed)

| ID | Title | Impact | Notes |
|----|-------|--------|-------|
| REQ_NETIF_TUNNEL_1 | QEMU UART Network Bridge | covered by new SPEC | Will be linked from SPEC_NETIF_UART_ARCH_1 |
| REQ_NETIF_TUNNEL_2 | Packet Encapsulation | covered by new SPEC | Will be linked from SPEC_NETIF_UART_PROTO_1 |
| REQ_NETIF_TUNNEL_3 | Host-Side Bridge Script | covered by new SPEC | Will be linked from SPEC_NETIF_UART_BRIDGE_1 |
| REQ_NETIF_TUNNEL_4 | DHCP Client Support | covered by new SPEC | Will be linked from SPEC_NETIF_UART_DHCP_1 |
| REQ_NETIF_TUNNEL_5 | Conditional Compilation | covered by new SPEC | Will be linked from SPEC_NETIF_UART_COND_1 |
| REQ_NETIF_TUNNEL_DOC_1 | Emulation Setup Documentation | covered by new SPEC | Will be linked from SPEC_NETIF_UART_DOC_1 |
| REQ_NETIF_TUNNEL_NF_1 | Tunnel Throughput | covered by new SPEC | Will be linked from SPEC_NETIF_UART_PERF_1 |
| REQ_NETIF_TUNNEL_NF_2 | Packet Loss Handling | covered by new SPEC | Will be linked from SPEC_NETIF_UART_PERF_1 |

### No new Requirements

All requirements already exist and are approved. This is a documentation catch-up — not
a design change.

### Horizontal Check (MECE)

- ✅ No contradictions with existing Requirements
- ✅ No redundancies — REQs are already approved
- ✅ All 8 uncovered REQs will be addressed by new SPECs

---

## Level 2: Design

**Status**: ✅ completed

### Impacted Design Elements

| ID | Title | Impact | Notes |
|----|-------|--------|-------|
| SPEC_ARCH_NETIF_1 | UART Tunnel Netif Component (arch overview) | no change needed | Already approved; new spec_netif_tunnel.rst provides component detail |

### New Design Elements

All SPECs are derived from code review of:
- `main/components/netif_uart_tunnel/netif_uart_tunnel_sim.c`
- `main/components/netif_uart_tunnel/netif_uart_tunnel_sim.h`
- `tools/qemu/network/serial_tun_bridge.py`
- `main/components/netif_uart_tunnel/README.md`

#### SPEC_NETIF_UART_ARCH_1 — Component Architecture

```rst
.. spec:: UART Tunnel Component Architecture
   :id: SPEC_NETIF_UART_ARCH_1
   :links: REQ_NETIF_TUNNEL_1, REQ_NETIF_TUNNEL_5
   :status: approved
   :tags: netif, qemu, uart, architecture

   **Description:** The ``netif_uart_tunnel`` component implements a lwIP network interface
   that tunnels IP packets over UART1, providing full TCP/IP stack functionality in QEMU
   emulation. The component is conditionally compiled only for ``CONFIG_IDF_TARGET_ESP32_QEMU``
   targets.

   **Component Location:** ``main/components/netif_uart_tunnel/``

   **Key Files:**

   - ``netif_uart_tunnel_sim.c`` — Implementation (``_sim`` suffix marks simulator-only)
   - ``netif_uart_tunnel_sim.h`` — Public API
   - ``CMakeLists.txt`` — Build integration

   **Threading Model:**

   - ``uart_rx_task`` (FreeRTOS task, priority 2, stack 4096 bytes): reads UART1 frames
     and injects into lwIP via ``esp_netif_receive``
   - TX path: lwIP calls ``netif_linkoutput`` synchronously in ``tcpip_thread`` context

   **Data Flow:**

   .. code-block:: text

      ┌─────────────────────────────────────────────────────────┐
      │  ESP32 (QEMU)                                           │
      │  Application → lwIP → etharp_output → netif_linkoutput │
      │                                         │               │
      │                                       UART1 TX (GPIO17) │
      └─────────────────────────────────────────────────────────┘
               ↕  Unix socket (temp/esp32-uart1.sock)
      ┌─────────────────────────────────────────────────────────┐
      │  Host (Linux)                                           │
      │  serial_tun_bridge.py                                   │
      │  UART socket ↔ TUN device (tun0)                       │
      │  tun0: 192.168.100.1/24                                 │
      └─────────────────────────────────────────────────────────┘

   **UART Configuration (hardcoded in implementation):**

   - Port: UART1
   - TX pin: GPIO17, RX pin: GPIO16
   - Baud rate: 115200
   - Buffer: 2048 bytes (RX and TX)

   **Network Configuration (default):**

   - ESP32 IP: 192.168.100.2
   - Gateway/Host: 192.168.100.1
   - Netmask: 255.255.255.0
   - ESP32 MAC: 02:00:00:00:00:02 (static, assigned in init)

   **Known Limitation:** This component is QEMU-only. It MUST NOT be linked in hardware
   builds. See ``SPEC_NETIF_UART_COND_1`` for build guard details.
```

#### SPEC_NETIF_UART_PROTO_1 — Wire Protocol (Length-Prefix Framing)

```rst
.. spec:: UART Wire Protocol — Length-Prefix Framing
   :id: SPEC_NETIF_UART_PROTO_1
   :links: REQ_NETIF_TUNNEL_2, REQ_NETIF_TUNNEL_NF_2
   :status: approved
   :tags: netif, qemu, protocol, framing

   **Description:** IP packets (wrapped in Ethernet frames) are transported over UART1
   using a simple 2-byte length-prefix framing protocol.

   **Frame Format:**

   .. code-block:: text

      ┌─────────────────┬────────────────────────────────────┐
      │  LENGTH (2 B)   │  DATA (N bytes, Ethernet frame)    │
      │  big-endian     │  N = 14 (eth hdr) + IP payload     │
      │  uint16_t       │  max N = 1500 bytes (MTU)          │
      └─────────────────┴────────────────────────────────────┘

   **Ethernet Encapsulation:**

   The DATA field is a complete Ethernet frame (not raw IP). lwIP operates as an
   Ethernet interface (``NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET``), so it produces
   and consumes full Ethernet frames including the 14-byte header.

   The Python bridge strips/adds the 14-byte Ethernet header when interfacing with
   the TUN device (which handles raw IP packets).

   **Framing Rules:**

   - LENGTH is transmitted as 2 bytes, big-endian, immediately before DATA
   - LENGTH encodes the byte count of DATA only (not including the 2-byte header itself)
   - Receiver MUST read exactly 2 bytes for LENGTH, then exactly LENGTH bytes for DATA
   - Frames with ``LENGTH == 0`` or ``LENGTH > 1500`` are invalid; receiver MUST flush
     UART and resync
   - No checksum or CRC — TCP handles end-to-end integrity

   **TX Path (ESP32 → Host):**

   1. lwIP calls ``netif_linkoutput`` with pbuf chain
   2. pbuf chain is copied into contiguous ``frame_buf`` via ``pbuf_copy_partial``
   3. 2-byte length header is written to UART1 via ``uart_write_bytes``
   4. Frame data is written to UART1 via ``uart_write_bytes``
   5. ``frame_buf`` is freed

   **RX Path (Host → ESP32):**

   1. ``uart_rx_task`` reads 2-byte length header with ``UART_READ_TIMEOUT_MS = 100 ms``
   2. Reads LENGTH bytes of frame data with 1 s timeout
   3. Passes frame to lwIP via ``esp_netif_receive``
   4. lwIP calls ``tcpip_input`` → processes Ethernet frame

   **Error Recovery:**

   - Invalid LENGTH: log warning, ``uart_flush()``, delay 100 ms, retry
   - Incomplete frame data (timeout): log warning, discard, retry
   - No data within timeout: ``vTaskDelay(10 ms)``, retry (avoids busy-loop)
```

#### SPEC_NETIF_UART_BRIDGE_1 — Host-Side Python Bridge

```rst
.. spec:: Host-Side Serial-TUN Bridge Script
   :id: SPEC_NETIF_UART_BRIDGE_1
   :links: REQ_NETIF_TUNNEL_3, REQ_NETIF_TUNNEL_DOC_1
   :status: approved
   :tags: netif, qemu, python, bridge, tooling

   **Description:** ``tools/qemu/network/serial_tun_bridge.py`` is the host-side
   component that bridges QEMU UART1 to the Linux TUN network device, giving the
   simulated ESP32 real network connectivity.

   **Script Location:** ``tools/qemu/network/serial_tun_bridge.py``

   **Prerequisites:**

   - Linux host (TUN device creation requires ``/dev/net/tun``)
   - Root privileges (for TUN device ioctl)
   - Python ≥ 3.8
   - Optional: ``pytun`` package (falls back to manual ``ioctl`` if absent)

   **Architecture:**

   .. code-block:: text

      QEMU UART1 (Unix socket at temp/esp32-uart1.sock)
           ↕ Ethernet frames with length-prefix
      SerialTunBridge.serial_to_tun() / tun_to_serial()
           ↕ Raw IP packets (Ethernet header stripped/added)
      TUN device tun0 (192.168.100.1/24)
           ↕
      Linux host network stack

   **TUN Device:**

   - Name: ``tun0``
   - Host IP: ``192.168.100.1``
   - Netmask: ``255.255.255.0``
   - MTU: 1500 bytes
   - Mode: IFF_TUN (raw IP, no packet info header)

   **Ethernet/IP Translation:**

   - **serial → TUN**: Strip 14-byte Ethernet header, write raw IP to TUN
   - **TUN → serial**: Read raw IP, prepend Ethernet header
     (DST=``02:00:00:00:00:02``, SRC=``02:00:00:00:00:01``, EtherType=0x0800),
     send framed Ethernet frame to serial socket

   **Socket Connection:**

   - QEMU exposes UART1 as a Unix domain socket at ``{PROJECT_DIR}/temp/esp32-uart1.sock``
   - Bridge connects via ``AF_UNIX / SOCK_STREAM``
   - Auto-reconnects if socket disappears (QEMU restart)

   **Execution Modes:**

   - Default (verbose): ``sudo python3 serial_tun_bridge.py`` — INFO logs to console
   - Quiet: ``sudo python3 serial_tun_bridge.py --quiet`` — only errors to
     ``temp/tun_errors.log``
   - The ``run_qemu.sh`` script automatically starts bridge in quiet mode

   **Known Limitations:**

   - Linux only (TUN device creation uses Linux-specific ioctls)
   - Requires root/sudo
   - Single TUN device per host (``tun0`` name hardcoded)
   - Reconnects on QEMU restart but TUN device persists across reconnects
```

#### SPEC_NETIF_UART_DHCP_1 — IP Configuration and DHCP Integration

```rst
.. spec:: IP Configuration and DHCP Client Integration
   :id: SPEC_NETIF_UART_DHCP_1
   :links: REQ_NETIF_TUNNEL_4
   :status: approved
   :tags: netif, qemu, dhcp, ip

   **Description:** The tunnel interface uses static IP configuration (not DHCP client)
   as implemented. The requirement ``REQ_NETIF_TUNNEL_4`` specifies DHCP support as a
   goal; the current implementation satisfies the intent via static configuration that
   mirrors DHCP-assigned values.

   **Implemented Behaviour:**

   - DHCP client is explicitly **stopped** via ``esp_netif_dhcpc_stop()`` in ``netif_uart_tunnel_init``
   - Static IP is set via ``esp_netif_set_ip_info()`` with caller-provided config
   - Caller in ``main.c`` passes: IP ``192.168.100.2``, GW ``192.168.100.1``, mask ``255.255.255.0``
   - IP info is logged to console at INFO level after init

   **Static ARP Entry:**

   A static ARP entry is added for the gateway MAC (``02:00:00:00:00:01``) to avoid
   ARP request/response cycles that would fail in point-to-point UART tunnels:

   .. code-block:: c

      struct eth_addr gw_mac = {{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}};
      etharp_add_static_entry(&gw_ip, &gw_mac);

   **Rationale for Static Config:** A UART tunnel is a point-to-point link; DHCP
   discovery over it would require the host-side bridge to run a DHCP server. Static
   configuration is simpler, deterministic, and sufficient for emulation purposes.

   **AC Coverage vs REQ_NETIF_TUNNEL_4:**

   - AC-1 (support lwIP DHCP client): ⚠️ Not implemented — static IP used instead.
     Acceptable for emulation; DHCP adds complexity without benefit.
   - AC-2 (DHCP discovery via tunnel): Not applicable with static config.
   - AC-3 (obtain IP from host DHCP): Not implemented — static IP is equivalent.
   - AC-4 (log IP to console): ✅ Implemented via ``ESP_LOGI`` in ``netif_uart_tunnel_init``.

   **Note:** AC-1 through AC-3 of ``REQ_NETIF_TUNNEL_4`` are not met by the current
   implementation. The requirement should be revisited if dynamic IP assignment becomes
   necessary. For the current QEMU emulation use case, static IP is the preferred approach.
```

#### SPEC_NETIF_UART_COND_1 — Conditional Compilation Guard

```rst
.. spec:: Conditional Compilation — QEMU-Only Build Guard
   :id: SPEC_NETIF_UART_COND_1
   :links: REQ_NETIF_TUNNEL_5
   :status: approved
   :tags: netif, qemu, build, kconfig

   **Description:** The tunnel component is conditionally included in the build using
   CMake and Kconfig guards, ensuring zero overhead on hardware builds.

   **CMakeLists.txt Guard:**

   The component's ``CMakeLists.txt`` uses ``idf_build_get_property`` to check the
   target and only registers the component for QEMU builds:

   .. code-block:: cmake

      idf_build_get_property(idf_target IDF_TARGET)
      if("${idf_target}" STREQUAL "esp32")
          # Only build for QEMU (target is still esp32, but checked via Kconfig below)
      endif()

   **Kconfig Guard (source-level):**

   The header and caller code use ``#ifdef CONFIG_IDF_TARGET_ESP32_QEMU`` (or the
   equivalent CMake variable ``CONFIG_TARGET_EMULATOR``) to exclude the component
   from hardware builds.

   **Application-Level Guard in ``main.c``:**

   .. code-block:: c

      #ifdef CONFIG_TARGET_EMULATOR
          netif_uart_tunnel_init(&tunnel_config);
      #else
          // WiFi init for real hardware
          wifi_manager_init();
      #endif

   **Result:** On hardware builds:

   - Component source files are not compiled
   - No symbols are linked
   - Zero flash/RAM overhead
   - ``netif_uart_tunnel_init`` call is preprocessed away

   **Kconfig Options (from README):**

   - ``CONFIG_NETIF_UART_TUNNEL_ENABLED`` — Enable tunnel (default: y for QEMU)
   - ``CONFIG_NETIF_UART_TUNNEL_UART_NUM`` — UART port (default: 1)
   - ``CONFIG_NETIF_UART_TUNNEL_BAUD_RATE`` — Baud rate (default: 921600 per README, 115200 per code)

   **Note:** The baud rate discrepancy between README (921600) and implementation
   constant ``UART_BAUD_RATE = 115200`` in ``netif_uart_tunnel_sim.c`` is a known
   inconsistency. The code value (115200) is what actually runs.
```

#### SPEC_NETIF_UART_PERF_1 — Performance Characteristics and Known Limitations

```rst
.. spec:: Performance Characteristics and Known Limitations
   :id: SPEC_NETIF_UART_PERF_1
   :links: REQ_NETIF_TUNNEL_NF_1, REQ_NETIF_TUNNEL_NF_2
   :status: approved
   :tags: netif, qemu, performance, limitations

   **Description:** Documents measured and expected performance of the UART tunnel,
   including known limitations for use in emulation contexts.

   **Throughput:**

   - Actual baud rate: 115200 bps (``UART_BAUD_RATE`` constant in implementation)
   - Theoretical raw throughput: ~14.4 KB/s (115200 / 8 bits, no overhead)
   - With framing overhead (2 bytes per packet): ~14.3 KB/s effective
   - ``SPEC_ARCH_NETIF_1`` documents ~10 KB/s measured throughput
   - **Does NOT meet REQ_NETIF_TUNNEL_NF_1 AC-1 (100 KB/s minimum)**
   - Baud rate is configurable; increasing to 921600 would yield ~115 KB/s
   - README documents 921600 as intended baud rate (code still uses 115200)

   **Packet Loss Handling:**

   - Invalid LENGTH detected: UART flushed, 100 ms delay, retry — satisfies
     ``REQ_NETIF_TUNNEL_NF_2 AC-1``
   - Incomplete frame data: warning logged, packet discarded — satisfies
     ``REQ_NETIF_TUNNEL_NF_2 AC-1``
   - TCP layer recovery from packet loss: inherent in TCP — satisfies
     ``REQ_NETIF_TUNNEL_NF_2 AC-2``
   - Packet loss statistics logging: ``s_rx_count`` and ``s_tx_count`` debug counters
     exposed via ``ESP_LOGD`` — satisfies ``REQ_NETIF_TUNNEL_NF_2 AC-3``

   **RX Task Design:**

   - Stack: 4096 bytes
   - Priority: 2 (lower than display_logic at 3 to avoid blocking)
   - Read timeout: 100 ms for length header; 1 s for frame data
   - 1-second startup delay after UART flush (stability wait)
   - 10 ms yield on timeout (avoids busy-loop when no traffic)

   **Known Limitations:**

   1. **QEMU-only**: TUN device creation requires Linux and root privileges
   2. **115200 baud**: Below 100 KB/s NF target (fixable by increasing baud rate)
   3. **Static IP only**: No DHCP client (see ``SPEC_NETIF_UART_DHCP_1``)
   4. **Linux host required**: ``serial_tun_bridge.py`` uses Linux TUN ioctls
   5. **Root required**: TUN device creation requires ``sudo``
   6. **Single instance**: Module uses static state; ``netif_uart_tunnel_init`` returns
      ``ESP_ERR_INVALID_STATE`` on second call
   7. **No graceful task stop**: ``netif_uart_tunnel_deinit`` calls ``vTaskDelete``
      on RX task (no cooperative shutdown signal)
```

#### SPEC_NETIF_UART_DOC_1 — Emulation Setup Documentation Pointer

```rst
.. spec:: Emulation Setup Documentation
   :id: SPEC_NETIF_UART_DOC_1
   :links: REQ_NETIF_TUNNEL_DOC_1
   :status: approved
   :tags: netif, qemu, documentation

   **Description:** Documents where emulation setup instructions are maintained
   and what they must cover.

   **Documentation Locations:**

   - ``main/components/netif_uart_tunnel/README.md`` — Component-level README with
     quick-start, configuration, usage, troubleshooting
   - ``tools/qemu/run_qemu.sh`` — QEMU launch script (self-documenting via comments)
   - ``tools/qemu/network/serial_tun_bridge.py`` — Bridge script with embedded docstring
   - ``docs/90_guides/`` — Developer guides (see ``switching-dev-modes.rst``)

   **Coverage by Requirement AC:**

   - ``REQ_NETIF_TUNNEL_DOC_1 AC-1`` (QEMU build/install): Covered in README and
     ``tools/qemu/`` scripts
   - ``REQ_NETIF_TUNNEL_DOC_1 AC-2`` (TAP/TUN interface setup): Covered in
     ``serial_tun_bridge.py`` docstring and README
   - ``REQ_NETIF_TUNNEL_DOC_1 AC-3`` (example commands): Covered in README
     troubleshooting section and ``run_qemu.sh``
   - ``REQ_NETIF_TUNNEL_DOC_1 AC-4`` (limitations vs hardware): Covered in
     README limitations section and ``SPEC_NETIF_UART_PERF_1`` above
```

### Horizontal Check (MECE)

- ✅ No contradictions with `SPEC_ARCH_NETIF_1` (intentionally kept as high-level overview)
- ✅ No redundancies — new SPECs add component detail, not re-state architectural overview
- ✅ All 8 REQs covered: TUNNEL_1→ARCH_1+COND_1, TUNNEL_2→PROTO_1, TUNNEL_3→BRIDGE_1, TUNNEL_4→DHCP_1, TUNNEL_5→COND_1, DOC_1→BRIDGE_1+DOC_1, NF_1→PERF_1, NF_2→PROTO_1+PERF_1
- ✅ Every new SPEC links to at least one REQ
- ⚠️ `SPEC_NETIF_UART_DHCP_1` documents a partial implementation gap vs AC-1..3 of `REQ_NETIF_TUNNEL_4`. This is intentionally documented as a known deviation; no code change is needed for this catch-up task.
- ⚠️ `SPEC_NETIF_UART_PERF_1` documents that 115200 baud does not meet the 100 KB/s NF target. The baud rate constant was increased to 921600 in the README but not in code. This deviation is documented.

### Decisions

- **Decision 1**: Use ID prefix `SPEC_NETIF_UART_` (not `SPEC_NETIF_TUN_`) to match
  the component name `netif_uart_tunnel` and be distinct from any future WiFi netif specs.
- **Decision 2**: Keep `SPEC_ARCH_NETIF_1` unchanged. It serves as the arch overview entry
  point. The new `spec_netif_tunnel.rst` provides the component detail layer.
- **Decision 3**: Document the DHCP gap in the spec rather than silently claiming compliance.
  Honest specs are more useful than optimistic ones.
- **Decision 4**: Document the 115200 vs 921600 baud rate inconsistency. This is a known
  issue in the code that should be fixed separately.

---

## Final Consistency Check

**Status**: ✅ passed

### Traceability Verification

| Requirement | SPEC | Complete? |
|-------------|------|-----------|
| REQ_NETIF_TUNNEL_1 | SPEC_NETIF_UART_ARCH_1 | ✅ |
| REQ_NETIF_TUNNEL_2 | SPEC_NETIF_UART_PROTO_1 | ✅ |
| REQ_NETIF_TUNNEL_3 | SPEC_NETIF_UART_BRIDGE_1 | ✅ |
| REQ_NETIF_TUNNEL_4 | SPEC_NETIF_UART_DHCP_1 | ✅ (with documented deviation) |
| REQ_NETIF_TUNNEL_5 | SPEC_NETIF_UART_COND_1 | ✅ |
| REQ_NETIF_TUNNEL_DOC_1 | SPEC_NETIF_UART_DOC_1 | ✅ |
| REQ_NETIF_TUNNEL_NF_1 | SPEC_NETIF_UART_PERF_1 | ✅ (with documented deviation) |
| REQ_NETIF_TUNNEL_NF_2 | SPEC_NETIF_UART_PROTO_1, SPEC_NETIF_UART_PERF_1 | ✅ |

### Issues Found

- ⚠️ **Baud rate inconsistency**: README says 921600, code uses 115200. Documented in
  `SPEC_NETIF_UART_PERF_1`. Separate fix task recommended.
- ⚠️ **DHCP not implemented**: `REQ_NETIF_TUNNEL_4` AC-1..3 not met. Documented in
  `SPEC_NETIF_UART_DHCP_1`. Static IP sufficient for emulation; no immediate action needed.

### Sign-off

- [x] All levels completed (no ⚠️ DEPRECATED markers remaining)
- [x] All conflicts resolved (deviations documented, not hidden)
- [x] Traceability verified (all 8 REQs covered)
- [x] Ready for implementation (spec writing + index update)

---

## Implementation Instructions

The Implement Agent should create the following files:

### 1. New file: `docs/12_design/spec_netif_tunnel.rst`

File content: combine all SPEC blocks above into a proper RST document following
the structure of `docs/12_design/spec_web_server.rst` or similar.

Structure:
```
UART Tunnel Network Interface Design Specification
==================================================

[intro paragraph]

Component Architecture
----------------------
[SPEC_NETIF_UART_ARCH_1 block]

Wire Protocol
-------------
[SPEC_NETIF_UART_PROTO_1 block]

Host-Side Bridge
----------------
[SPEC_NETIF_UART_BRIDGE_1 block]

IP Configuration
----------------
[SPEC_NETIF_UART_DHCP_1 block]

Conditional Compilation
-----------------------
[SPEC_NETIF_UART_COND_1 block]

Performance and Limitations
----------------------------
[SPEC_NETIF_UART_PERF_1 block]

Documentation
-------------
[SPEC_NETIF_UART_DOC_1 block]

Traceability
------------
[needtable + needflow for REQ_NETIF_TUNNEL_1]
```

### 2. Update: `docs/12_design/index.rst`

Add `spec_netif_tunnel` to the toctree after `spec_high_level_architecture`.

Update the "Design Hierarchy" section to mention:
- `SPEC_NETIF_UART_*` — UART Tunnel Netif Component design

---

## Appendix: Code Review Summary

### `netif_uart_tunnel_sim.c` key facts

| Fact | Value |
|------|-------|
| UART port | UART_NUM_1 |
| TX pin | GPIO17 |
| RX pin | GPIO16 |
| Baud rate | 115200 (constant) |
| Frame format | 2-byte BE length + Ethernet frame data |
| Max frame | 1500 bytes |
| RX task stack | 4096 bytes |
| RX task priority | 2 |
| Read timeout (length) | 100 ms |
| Read timeout (data) | 1000 ms |
| Startup delay | 1000 ms after uart_flush |
| MAC address | 02:00:00:00:00:02 (static) |
| IP stack mode | Ethernet (NETIF_FLAG_ETHARP + NETIF_FLAG_ETHERNET) |
| Default IP | 192.168.100.2 (set by caller) |
| DHCP | Disabled (static IP) |
| Static ARP for GW | 02:00:00:00:00:01 |

### `serial_tun_bridge.py` key facts

| Fact | Value |
|------|-------|
| Socket path | `{PROJECT_DIR}/temp/esp32-uart1.sock` |
| Socket type | AF_UNIX / SOCK_STREAM |
| TUN device | tun0 |
| Host IP | 192.168.100.1/24 |
| Host MAC | 02:00:00:00:00:01 |
| ESP32 MAC | 02:00:00:00:00:02 |
| Frame direction TUN→serial | Adds 14-byte Ethernet header to IP packet |
| Frame direction serial→TUN | Strips 14-byte Ethernet header |
| Auto-reconnect | Yes (on serial disconnect) |
| Quiet mode | --quiet flag, errors to temp/tun_errors.log |

---

*Generated by syspilot Change Agent*
