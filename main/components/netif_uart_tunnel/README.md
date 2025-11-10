# Network Interface UART Tunnel Component

## Purpose

Provides network connectivity for ESP32 running in QEMU by tunneling network packets over UART.

This component is **required for QEMU emulation** with network support.

## How It Works

1. Creates a virtual network interface on the ESP32
2. Bridges UART1 to the network interface
3. Host-side Python script (`tools/qemu/serial_tun_bridge.py`) creates TUN device
4. Network packets flow: ESP32 ↔ UART1 ↔ TUN device ↔ Host network

## Configuration

### Kconfig Options

- `CONFIG_NETIF_UART_TUNNEL_ENABLED` - Enable UART tunnel (default: y)
- `CONFIG_NETIF_UART_TUNNEL_UART_NUM` - UART port number (default: 1)
- `CONFIG_NETIF_UART_TUNNEL_BAUD_RATE` - Baud rate (default: 921600)

### Network Settings

Default IP configuration (matches host TUN device):

- IP Address: `192.168.5.2`
- Gateway: `192.168.5.1`
- Netmask: `255.255.255.0`

## Usage

### In QEMU

The component is automatically initialized when running in QEMU:

```c
#include "netif_uart_tunnel.h"

// Initialize UART network tunnel
esp_err_t ret = netif_uart_tunnel_init();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "UART tunnel init failed");
}
```

### Host-Side Setup

The QEMU run scripts automatically start the bridge:

```bash
# Automatically started by run-qemu-*.sh scripts
sudo python3 tools/qemu/serial_tun_bridge.py /dev/pts/X
```

## Network Access

Once initialized, the ESP32 in QEMU has full network access:

```c
// Use standard ESP-IDF networking
esp_http_client_config_t config = {
    .url = "http://example.com",
};
esp_http_client_handle_t client = esp_http_client_init(&config);
```

## Testing Connectivity

```bash
# From QEMU ESP32, ping the host gateway
ping 192.168.5.1

# From host, ping the ESP32
ping 192.168.5.2
```

## Dependencies

- `esp_netif` - Network interface abstraction
- `driver` - UART driver
- `lwip` - TCP/IP stack

## Limitations

- **QEMU only** - Not functional on real hardware
- **Linux host required** - TUN device creation needs Linux
- **Root privileges** - TUN device creation requires sudo

## Troubleshooting

### No network connectivity in QEMU

1. Check UART bridge is running:
   ```bash
   ps aux | grep serial_tun_bridge.py
   ```

2. Verify TUN device exists:
   ```bash
   ip addr show tap0
   ```

3. Check ESP32 IP configuration:
   ```c
   esp_netif_ip_info_t ip_info;
   esp_netif_get_ip_info(netif, &ip_info);
   ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&ip_info.ip));
   ```

### UART tunnel initialization fails

- Verify `CONFIG_NETIF_UART_TUNNEL_ENABLED=y` in sdkconfig
- Check UART1 is not used by other components
- Ensure sufficient heap memory available

## See Also

- [QEMU Run Scripts](../../../tools/qemu/)
- [Serial TUN Bridge Script](../../../tools/qemu/serial_tun_bridge.py)
- [Network Utilities](../../../tools/network/)
