# QEMU Network Internals - Deep Dive

This document provides detailed technical information about the UART-based IP tunnel implementation for ESP32 QEMU emulation.

## Architecture Overview

```
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
```

## Frame Format (Over UART)

```
┌───────────────┬────────────────────────────────────────────┐
│ Frame Length  │           Ethernet Frame                   │
│   (2 bytes)   │         (14-byte header + IP)              │
│  Big Endian   │                                            │
├───────────────┼────────────────────────────────────────────┤
│  [HI] [LO]    │ [DST_MAC:6][SRC_MAC:6][TYPE:2][IP PACKET]  │
└───────────────┴────────────────────────────────────────────┘

Example ICMP Echo Request (98 bytes total):
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
```

## Initialization Sequence

### 1. System Startup

```
┌──────────────────────────────────────────────────────────────┐
│                  ESP32 QEMU Boot                             │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  Initialize Network Stack (wifi_manager_sim.c)               │
│     - esp_netif_init()                                       │
│     - esp_event_loop_create_default()                        │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  Initialize UART Tunnel Driver (netif_uart_tunnel_sim.c)    │
│                                                              │
│  a) Hardware Setup:                                          │
│     - Configure UART1: 115200 baud, GPIO 17/16              │
│     - uart_driver_install() with 2KB RX/TX buffers          │
│                                                              │
│  b) Create esp_netif:                                        │
│     - ESP_NETIF_INHERENT_DEFAULT_ETH() config               │
│     - esp_netif_new()                                        │
│                                                              │
│  c) Configure Static IP:                                     │
│     - IP: 192.168.100.2                                      │
│     - Gateway: 192.168.100.1                                 │
│     - Netmask: 255.255.255.0                                 │
│                                                              │
│  d) Direct lwIP Integration:                                 │
│     - Get lwIP netif handle                                  │
│     - Set MAC: 02:00:00:00:00:02                             │
│     - Set flags: ETHARP | ETHERNET | BROADCAST               │
│     - Register linkoutput callback                           │
│     - Set as default netif                                   │
│     - Add static ARP entry for gateway                       │
│                                                              │
│  e) Start RX Task:                                           │
│     - xTaskCreate(uart_rx_task, priority=5)                  │
│                                                              │
│  f) Bring Interface Up:                                      │
│     - netif_set_up()                                         │
│     - netif_set_link_up()                                    │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  Start TUN Bridge on Host                                    │
│     - Create tun0 device                                     │
│     - Configure IP: 192.168.100.1/24                         │
│     - Connect to TCP:5556 (QEMU UART1)                       │
│     - Start bidirectional forwarding                         │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  Network Ready                                               │
│     - Interface UP, LINK_UP                                  │
│     - Routes configured                                      │
│     - ARP cache populated                                    │
└──────────────────────────────────────────────────────────────┘
```

## Packet Flow - PING Example

### Inbound Path (Host → ESP32)

```
1. Host executes: ping 192.168.100.2
   └─▶ Kernel creates ICMP Echo Request
       └─▶ Routes to tun0 (192.168.100.0/24 network)

2. TUN Bridge (serial_tun_bridge.py)
   ├─▶ read() from tun0 gets raw IP packet
   ├─▶ Prepends Ethernet header:
   │   - Dst MAC: 02:00:00:00:00:02 (ESP32)
   │   - Src MAC: 02:00:00:00:00:01 (Host)
   │   - EtherType: 0x0800 (IPv4)
   ├─▶ Prepends 2-byte length (big-endian)
   └─▶ write() to TCP socket (QEMU UART1)

3. QEMU Serial Device
   └─▶ Forwards bytes to emulated UART1 RX FIFO

4. ESP32 uart_rx_task (netif_uart_tunnel_sim.c)
   ├─▶ uart_read_bytes() reads 2-byte length header
   ├─▶ Validates: 0 < len <= MAX_FRAME_SIZE
   ├─▶ uart_read_bytes() reads complete Ethernet frame
   ├─▶ Allocates lwIP pbuf
   ├─▶ Copies frame to pbuf
   └─▶ Calls netif->input(pbuf, netif)
       └─▶ This is ethernet_input() [KEY: Direct lwIP call]

5. lwIP ethernet_input()
   ├─▶ Parses Ethernet header
   ├─▶ Checks destination MAC (matches!)
   ├─▶ Checks EtherType (0x0800 = IPv4)
   ├─▶ Strips Ethernet header (14 bytes)
   └─▶ Calls ip4_input() with IP packet

6. lwIP ip4_input()
   ├─▶ Validates IP header checksum
   ├─▶ Checks destination IP (192.168.100.2 - matches!)
   ├─▶ Identifies protocol (ICMP = 1)
   └─▶ Calls icmp_input()

7. lwIP icmp_input()
   ├─▶ Validates ICMP checksum
   ├─▶ Identifies Echo Request (type=8)
   ├─▶ Creates Echo Reply (type=0)
   ├─▶ Swaps src/dst IPs
   └─▶ Calls ip4_output_if() to send response
```

### Outbound Path (ESP32 → Host)

```
8. lwIP ip4_output_if()
   ├─▶ Looks up route for 192.168.100.1
   ├─▶ Finds default gateway on tun_netif
   ├─▶ Checks ARP cache for 192.168.100.1
   ├─▶ Finds static entry: 02:00:00:00:00:01
   ├─▶ Prepends Ethernet header
   └─▶ Calls netif->linkoutput(netif, pbuf)

9. uart_linkoutput() (netif_uart_tunnel_sim.c)
   ├─▶ Calculates total length from pbuf chain
   ├─▶ Allocates temporary buffer
   ├─▶ Copies all pbuf segments to contiguous buffer
   ├─▶ Prepends 2-byte length header (big-endian)
   ├─▶ uart_write_bytes() - length header
   ├─▶ uart_write_bytes() - Ethernet frame
   └─▶ Frees temporary buffer

10. ESP32 UART1 → QEMU
    └─▶ Bytes sent to TCP socket

11. TUN Bridge (serial_tun_bridge.py)
    ├─▶ read() from TCP gets length header
    ├─▶ read() gets complete Ethernet frame
    ├─▶ Strips Ethernet header (14 bytes)
    ├─▶ Extracts IP packet
    └─▶ write() to tun0

12. Host Kernel
    ├─▶ Receives ICMP Echo Reply from tun0
    ├─▶ Matches with pending ping request
    └─▶ Displays: "64 bytes from 192.168.100.2: icmp_seq=1 ttl=64 time=X ms"
```

## Key Implementation Details

### Direct lwIP Integration

Unlike typical ESP-IDF network interfaces that use the full `esp_netif` abstraction, this implementation integrates directly with lwIP for better control:

```c
// Get direct access to lwIP netif structure
struct netif* lwip_netif = esp_netif_get_netif_impl(esp_netif_handle);

// Set input function directly (bypasses esp_netif RX path)
lwip_netif->input = ethernet_input;

// Set output function (called by lwIP for TX)
lwip_netif->linkoutput = uart_linkoutput;
```

**Why Direct Integration?**
- Simpler packet flow (fewer abstraction layers)
- Direct control over Ethernet frame handling
- Easier debugging (fewer indirect function calls)
- Better performance (reduced function call overhead)

### Static ARP Entry

To avoid ARP broadcasts (which don't work well over UART):

```c
ip4_addr_t gateway_ip;
IP4_ADDR(&gateway_ip, 192, 168, 100, 1);

struct eth_addr gateway_mac = {{0x02, 0x00, 0x00, 0x00, 0x00, 0x01}};

// Add permanent ARP entry
etharp_add_static_entry(&gateway_ip, &gateway_mac);
```

This tells lwIP: "192.168.100.1 always has MAC 02:00:00:00:00:01" without needing ARP requests.

### pbuf Management

lwIP uses packet buffers (pbufs) for zero-copy networking:

```c
// RX: Allocate pbuf and copy UART data
struct pbuf* p = pbuf_alloc(PBUF_RAW, frame_len, PBUF_POOL);
memcpy(p->payload, uart_buffer, frame_len);
netif->input(p, netif);  // lwIP takes ownership

// TX: pbuf may be chained (fragmented)
u16_t total_len = 0;
for (struct pbuf* q = p; q != NULL; q = q->next) {
    memcpy(buffer + total_len, q->payload, q->len);
    total_len += q->len;
}
```

### Error Handling

The implementation includes comprehensive error handling:

```c
// Frame length validation
if (frame_len == 0 || frame_len > MAX_FRAME_SIZE) {
    ESP_LOGE(TAG, "Invalid frame length: %d", frame_len);
    continue;  // Skip bad frame, keep running
}

// pbuf allocation failure
struct pbuf* p = pbuf_alloc(...);
if (p == NULL) {
    ESP_LOGE(TAG, "Failed to allocate pbuf");
    free(frame_buffer);
    continue;  // Skip this packet
}

// UART read timeout (handled by uart_read_bytes)
int len = uart_read_bytes(UART_NUM, buffer, size, pdMS_TO_TICKS(1000));
if (len <= 0) {
    ESP_LOGW(TAG, "UART read timeout or error");
    continue;
}
```

## Performance Characteristics

### Throughput

- **UART Speed**: 115200 baud = ~11.5 KB/s theoretical max
- **Actual HTTP**: ~8-10 KB/s (due to protocol overhead)
- **Ping Latency**: 3-8ms typical

### Bottlenecks

1. **UART Baud Rate**: Limited to 115200 (QEMU constraint)
2. **Frame Overhead**: 16 bytes per packet (2-byte length + 14-byte Ethernet header)
3. **Context Switching**: FreeRTOS task scheduling adds latency

### Optimization Opportunities

- Increase UART buffer sizes for burst traffic
- Use DMA for UART transfers (if QEMU supports it)
- Implement zero-copy where possible
- Batch small packets together

## Debugging Tips

### Enable Verbose Logging

In `netif_uart_tunnel_sim.c`:
```c
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
```

This will show every packet RX/TX:
```
D (12345) uart_tunnel: RX: Got 98 bytes
D (12345) uart_tunnel: RX: Ethernet dst=02:00:00:00:00:02 src=02:00:00:00:00:01 type=0x0800
D (12345) uart_tunnel: TX: Sending 98 bytes
```

### Monitor TUN Device

```bash
# Watch all traffic on tun0
sudo tcpdump -i tun0 -vvv -X

# Filter for specific traffic
sudo tcpdump -i tun0 icmp
sudo tcpdump -i tun0 tcp port 80
```

### Check lwIP Statistics

Add to your code:
```c
#include "lwip/stats.h"

void print_lwip_stats(void) {
    ESP_LOGI(TAG, "lwIP RX packets: %d", lwip_stats.link.recv);
    ESP_LOGI(TAG, "lwIP TX packets: %d", lwip_stats.link.xmit);
    ESP_LOGI(TAG, "lwIP errors: %d", lwip_stats.link.err);
}
```

### UART Traffic Analysis

```bash
# Connect to UART1 directly to see raw bytes
nc localhost 5556 | hexdump -C
```

## Comparison with Real Hardware

| Feature | Real ESP32 | QEMU Emulation |
|---------|------------|----------------|
| Network Interface | WiFi (802.11) | UART tunnel |
| Speed | ~1-10 Mbps | ~11 KB/s |
| AP Mode | Yes | Simulated via IP |
| STA Mode | Yes | Direct IP connectivity |
| DNS | Full support | Via host system |
| DHCP | Supported | Static IP only |
| Security | WPA2/WPA3 | No encryption (local) |

## Known Issues and Workarounds

### Issue: UART Buffer Overflow

**Symptom**: Packets dropped during high traffic
**Cause**: UART RX buffer (2KB) fills faster than task can read
**Workaround**: Increase buffer size in `uart_driver_install()`

### Issue: TCP Connections Timeout

**Symptom**: HTTP connections hang
**Cause**: lwIP TCP retransmission timeout too aggressive
**Workaround**: Tune lwIP TCP timers in `lwipopts.h`

### Issue: ARP Not Working

**Symptom**: Can't reach hosts beyond gateway
**Cause**: Static ARP only configured for gateway
**Solution**: Add more static ARP entries or enable full ARP

## Further Reading

- [lwIP Documentation](https://www.nongnu.org/lwip/)
- [ESP-IDF Network API Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/index.html)
- [QEMU Serial Device Documentation](https://www.qemu.org/docs/master/system/devices/serial.html)

## Related Documentation

- **[QEMU Emulator Guide](qemu-emulator.md)** - User-facing guide for running QEMU
- **[Debugging Guide](debugging.md)** - Using GDB with QEMU
- **[Development Setup](devcontainer.md)** - Setting up your environment

---

*Last Updated: October 5, 2025*  
*Status: Network fully functional with ping, HTTP, and TCP support*
