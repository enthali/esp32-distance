# QEMU Emulator Guide

The ESP32 Distance Sensor project includes full QEMU emulation support, allowing you to develop and test without physical hardware. The emulator includes complete network functionality via a UART-based IP tunnel.

## Quick Start

### Starting QEMU

```bash
# From project root
./tools/run-qemu-network.sh
```

This script automatically:
- ✅ Builds the project (incremental, fast)
- ✅ Starts the TUN network bridge
- ✅ Starts the HTTP proxy for web access
- ✅ Launches QEMU with GDB support
- ✅ Waits for debugger connection

### Accessing the Web Interface

Once QEMU is running:

```bash
# In your browser or terminal
curl http://192.168.100.2/
```

Or use the HTTP proxy:
```bash
curl http://localhost:8080/
```

The web interface should be accessible at `http://localhost:8080` in your browser.

## Architecture Overview

```
┌────────────────────────────────────────────────────────────┐
│                    Host System (Linux)                     │
│                                                            │
│  ┌──────────┐    ┌───────────┐    ┌──────────────────┐    │
│  │ Browser  │───▶│   Proxy   │───▶│   tun0 Device    │    │
│  │  :8080   │    │   :8080   │    │  192.168.100.1   │    │
│  └──────────┘    └───────────┘    └──────────────────┘    │
│                                             │              │
│                                             │              │
│                                    ┌────────▼─────────┐    │
│                                    │   TUN Bridge     │    │
│                                    │   (Python)       │    │
│                                    └────────┬─────────┘    │
│                                             │ TCP:5556     │
└─────────────────────────────────────────────┼──────────────┘
                                              │
                                              │ QEMU chardev
┌─────────────────────────────────────────────┼──────────────┐
│                       ESP32 QEMU            │              │
│                                             ▼              │
│  ┌────────────┐    ┌──────────┐    ┌──────────────┐       │
│  │    Web     │    │   lwIP   │    │    UART1     │       │
│  │   Server   │◄──►│  Stack   │◄──►│   Driver     │       │
│  │   :80      │    │ 192.168. │    │              │       │
│  │            │    │ 100.2/24 │    └──────────────┘       │
│  └────────────┘    └──────────┘                           │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

## Network Configuration

### IP Addresses

| Component | IP Address | Description |
|-----------|------------|-------------|
| Host TUN device | 192.168.100.1/24 | Gateway for emulated ESP32 |
| ESP32 QEMU | 192.168.100.2/24 | Emulated device IP |

### Ports

| Port | Protocol | Purpose |
|------|----------|---------|
| 5555 | TCP | QEMU UART0 (console/monitor) |
| 5556 | TCP | QEMU UART1 (IP tunnel) |
| 8080 | HTTP | Proxy to ESP32 web server |
| 3333 | TCP | GDB debug server |

## How It Works

### 1. UART-Based IP Tunnel

The emulator uses UART1 as a network interface:

1. **Ethernet Frame Encapsulation**: IP packets are wrapped in Ethernet frames
2. **Length Prefix Protocol**: Each frame is prefixed with 2-byte length (big-endian)
3. **UART Transport**: Frames are transmitted over UART1 (115200 baud)
4. **TUN Bridge**: Python script bridges UART ↔ TUN device

### 2. Network Stack

```
Application (Web Server)
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
```

### 3. Frame Format

```
┌───────────────┬──────────────────────────────────────────┐
│ Frame Length  │         Ethernet Frame                   │
│   (2 bytes)   │    (14-byte header + IP packet)          │
│  Big Endian   │                                          │
├───────────────┼──────────────────────────────────────────┤
│  [HI] [LO]    │ [DST_MAC:6][SRC_MAC:6][TYPE:2][IP DATA]  │
└───────────────┴──────────────────────────────────────────┘
```

**Example:** ICMP Echo Request (98 bytes)
```
Length: 0x00 0x62 (98 bytes)
Ethernet:
  Dst MAC: 02:00:00:00:00:02 (ESP32)
  Src MAC: 02:00:00:00:00:01 (Host)
  Type:    0x08 0x00 (IPv4)
IP Packet:
  Src IP: 192.168.100.1
  Dst IP: 192.168.100.2
  Protocol: ICMP
```

## Testing Network Connectivity

### Ping Test

```bash
# Ping the emulated ESP32
ping -c 4 192.168.100.2
```

Expected output:
```
64 bytes from 192.168.100.2: icmp_seq=1 ttl=64 time=5.2 ms
64 bytes from 192.168.100.2: icmp_seq=2 ttl=64 time=3.8 ms
```

### HTTP Test

```bash
# Direct access
curl http://192.168.100.2/

# Via proxy
curl http://localhost:8080/
```

### Monitor Network Traffic

```bash
# Watch TUN device traffic
sudo tcpdump -i tun0 -n

# Monitor UART traffic in QEMU logs
# Look for "RX:" and "TX:" messages
```

## Troubleshooting

### QEMU Won't Start

**Problem:** Script fails to start QEMU

**Solutions:**
```bash
# Check if QEMU is already running
ps aux | grep qemu

# Kill existing QEMU processes
./tools/stop_qemu.sh

# Rebuild and try again
idf.py build
./tools/run-qemu-network.sh
```

### Network Not Working

**Problem:** Can't ping or access web server

**Checks:**
```bash
# 1. Verify TUN device exists
ip addr show tun0

# 2. Check TUN bridge is running
ps aux | grep serial_tun_bridge

# 3. Check HTTP proxy is running
ps aux | grep http_proxy

# 4. Restart network stack
./tools/stop_qemu.sh
./tools/run-qemu-network.sh
```

### No UART Output

**Problem:** No logs from QEMU

**Solution:**
```bash
# Connect to UART0 console
nc localhost 5555

# Or use dedicated viewer
./tools/view_uart1.sh
```

### Port Already in Use

**Problem:** Error about port 5555 or 5556 already in use

**Solution:**
```bash
# Find process using the port
lsof -i :5555
lsof -i :5556

# Kill the process
kill <PID>

# Or use the stop script
./tools/stop_qemu.sh
```

## Advanced Usage

### Debugging Network Issues

Enable verbose logging in `netif_uart_tunnel_sim.c`:

```c
// Temporarily change log level
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
```

Then rebuild and watch detailed packet flow:
```bash
idf.py build
./tools/run-qemu-network.sh
```

### Custom Network Configuration

Edit `main/components/netif_uart_tunnel/netif_uart_tunnel_sim.c`:

```c
// Change IP address
#define ESP32_IP "192.168.100.2"
#define GATEWAY_IP "192.168.100.1"
#define NETMASK "255.255.255.0"
```

### Running Without GDB

To start QEMU without waiting for debugger:

```bash
# Edit run-qemu-network.sh and remove the -d flag
idf.py qemu \
    --qemu-extra-args="-serial tcp::5555,server,nowait -serial tcp::5556,server,nowait -nographic"
```

## Technical Deep Dive

For detailed information about the network implementation, packet flow, and lwIP integration, see [Network Internals](qemu-network-internals.md).

## Known Limitations

- **UART Speed**: Limited to 115200 baud (adequate for HTTP, slow for large transfers)
- **No WiFi Simulation**: Uses direct IP connectivity instead of WiFi AP/STA modes
- **Browser Caching**: Web interface may cache old versions (use Ctrl+F5 to refresh)
- **QEMU Performance**: Slower than real hardware, but sufficient for testing

## Next Steps

- **[Debugging Guide](debugging.md)** - Set breakpoints and step through code
- **[Network Internals](qemu-network-internals.md)** - Understand packet flow in detail
- **[Dev Container Setup](devcontainer.md)** - Configure your development environment
