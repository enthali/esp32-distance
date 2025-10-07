# Debugging Guide

The ESP32 Distance Sensor project supports full GDB debugging in both emulator (QEMU) and hardware modes. This guide focuses on QEMU debugging since it's now fully supported in GitHub Codespaces.

## Quick Start: Debugging in QEMU

### 1. Start QEMU Debug Server

The QEMU emulator automatically starts with GDB support enabled:

```bash
./tools/run-qemu-network.sh
```

This starts QEMU in debug mode, waiting for a debugger connection on port 3333.

### 2. Start Debugging in VS Code

Simply press **F5** or use the Debug panel:

1. Open the Debug view (Ctrl+Shift+D / Cmd+Shift+D)
2. Select "**QEMU: GDB**" from the dropdown
3. Click the green play button or press F5

VS Code will:

- ✅ Connect to QEMU's GDB server (port 3333)
- ✅ Load symbols from the built ELF file
- ✅ Break at `app_main()`
- ✅ Show full call stack and variables

## Debugging Features

### Breakpoints

Set breakpoints by clicking in the editor gutter (left of line numbers):

```c
void app_main(void)
{
    // Breakpoint here: Click in gutter at line number
    ESP_LOGI(TAG, "ESP32 Distance Sensor Starting...");
    
    // Conditional breakpoint: Right-click → Add Conditional Breakpoint
    if (distance < 10) {
        // Break only when distance < 10
    }
}
```

### Watch Variables

Monitor variables in real-time:

1. **Watch Panel**: Add expressions to watch
2. **Variables Panel**: Inspect local and global variables
3. **Hover**: Mouse over variables to see current values

### Call Stack

View the complete function call hierarchy:

```text
#0  distance_sensor_read() at distance_sensor.c:45
#1  sensor_task() at main.c:123
#2  vPortTaskWrapper() at port.c:168
#3  0x400d1234 in ?? ()
```

### Step Through Code

| Key | Action | Description |
|-----|--------|-------------|
| **F10** | Step Over | Execute current line, don't enter functions |
| **F11** | Step Into | Enter function calls |
| **Shift+F11** | Step Out | Continue until current function returns |
| **F5** | Continue | Run until next breakpoint |

## Debug Configuration

The project includes two debug configurations in `.vscode/launch.json`:

### QEMU: GDB (Recommended)

```json
{
    "name": "QEMU: GDB",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/build/distance.elf",
    "miDebuggerServerAddress": "localhost:3333",
    "cwd": "${workspaceFolder}",
    "setupCommands": [
        { "text": "target remote localhost:3333" },
        { "text": "file build/distance.elf" },
        { "text": "thbreak app_main" },
        { "text": "continue" }
    ]
}
```

**Features:**

- Connects to QEMU GDB server
- Breaks at `app_main()` automatically
- Full source-level debugging
- Works in GitHub Codespaces

### ESP32 Hardware (Alternative)

For debugging on real ESP32 hardware with JTAG adapter:

```json
{
    "name": "ESP32: OpenOCD",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/build/distance.elf",
    "miDebuggerServerAddress": "localhost:3333",
    "cwd": "${workspaceFolder}",
    "setupCommands": [
        { "text": "target remote localhost:3333" },
        { "text": "mon reset halt" },
        { "text": "flushregs" },
        { "text": "thbreak app_main" },
        { "text": "continue" }
    ]
}
```

**Requirements:**

- ESP-PROG or similar JTAG adapter
- OpenOCD running with appropriate configuration
- Hardware connection (see ESP-IDF JTAG debugging guide)

## Common Debugging Scenarios

### Debug Network Stack

Set breakpoints in network components:

```c
// In wifi_manager_sim.c
esp_err_t wifi_manager_init_sim(void)
{
    ESP_LOGI(TAG, "Initializing network stack...");  // Breakpoint here
    
    // Step through network initialization
    esp_err_t ret = netif_uart_tunnel_init(&tunnel_config);
    return ret;
}
```

### Debug UART Tunnel

Watch packet flow in the IP tunnel:

```c
// In netif_uart_tunnel_sim.c
static void uart_rx_task(void *arg)
{
    while (1) {
        // Breakpoint here to see incoming packets
        int len = uart_read_bytes(UART_NUM, len_buf, 2, portMAX_DELAY);
        
        // Add to watch: len_buf[0], len_buf[1]
    }
}
```

### Debug Web Server

Trace HTTP request handling:

```c
// In web_server.c
static esp_err_t root_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Handling root request");  // Breakpoint here
    
    // Watch variables: req->uri, req->method
    httpd_resp_send(req, index_html_start, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
```

### Debug Sensor Reading

Monitor distance measurements:

```c
// In distance_sensor.c
esp_err_t distance_sensor_read(uint16_t *distance_cm)
{
    // Breakpoint here
    gpio_set_level(TRIGGER_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIGGER_PIN, 0);
    
    // Watch: *distance_cm after measurement
    *distance_cm = calculated_distance;
    return ESP_OK;
}
```

## Advanced Debugging

### GDB Command Line

Access GDB directly for advanced commands:

```bash
# In another terminal
xtensa-esp32-elf-gdb build/distance.elf

# Connect to QEMU
(gdb) target remote localhost:3333

# Load symbols
(gdb) file build/distance.elf

# Set breakpoint
(gdb) break app_main

# Continue execution
(gdb) continue

# Print variables
(gdb) print distance_cm

# Backtrace
(gdb) bt

# Examine memory
(gdb) x/16xb 0x3ffb0000
```

### Memory Inspection

View memory regions:

```gdb
# Check heap status
(gdb) call heap_caps_print_heap_info(MALLOC_CAP_8BIT)

# Inspect task stack
(gdb) info threads
(gdb) thread 2
(gdb) bt
```

### FreeRTOS Task Debugging

List all tasks:

```gdb
(gdb) info threads

# Switch to specific task
(gdb) thread 3

# View task stack
(gdb) bt full
```

## Debugging Tips

### 1. Enable Verbose Logging

Before debugging, increase log verbosity:

```c
// In component file
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Or in sdkconfig
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=y
```

### 2. Use Assertions

Add runtime checks:

```c
#include "esp_assert.h"

void process_data(uint8_t *buffer) {
    assert(buffer != NULL);
    ESP_ERROR_CHECK(process_buffer(buffer));
}
```

### 3. Watchpoints

Break when variable changes:

```gdb
(gdb) watch distance_cm
(gdb) continue
# Breaks when distance_cm is modified
```

### 4. Conditional Breakpoints

Break only when condition is true:

```gdb
(gdb) break distance_sensor_read if distance_cm < 10
```

Or in VS Code: Right-click breakpoint → Edit Breakpoint → Add condition

## Troubleshooting

### Debugger Won't Connect

**Problem:** VS Code can't connect to GDB server

**Solutions:**

```bash
# 1. Check if QEMU is running
ps aux | grep qemu

# 2. Verify GDB port is open
nc -zv localhost 3333

# 3. Restart QEMU
./tools/stop_qemu.sh
./tools/run-qemu-network.sh

# 4. Wait for QEMU to be ready (script does this automatically)
```

### No Symbols / Source Code

**Problem:** Debugger shows assembly instead of C code

**Solutions:**

```bash
# 1. Ensure project is built with debug symbols
idf.py menuconfig
# Component config → Compiler options → Optimization Level → Debug (-Og)

# 2. Rebuild
idf.py fullclean
idf.py build

# 3. Verify ELF file exists
ls -lh build/distance.elf
```

### Breakpoints Not Working

**Problem:** Breakpoints are ignored or show as "unverified"

**Solutions:**

- Ensure file is actually compiled (not excluded by #ifdef)
- Check optimization level (too high optimization can skip code)
- Verify you're debugging the correct build
- Try `thbreak` (temporary hardware breakpoint) instead

### Slow Debugging

**Problem:** Stepping through code is very slow

**This is normal in QEMU!** QEMU emulation is slower than real hardware.

**Tips to improve:**

- Use breakpoints instead of stepping
- Run to cursor (Right-click → Run to Cursor)
- Skip uninteresting functions with Step Over

## Next Steps

- **[QEMU Emulator Guide](qemu-emulator.md)** - Learn more about QEMU setup
- **[Network Internals](qemu-network-internals.md)** - Debug network issues
- **ESP-IDF JTAG Debugging** - [Official Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/jtag-debugging/)

## Resources

- [GDB Cheat Sheet](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf)
- [ESP-IDF Debugging Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/jtag-debugging/)
- [VS Code Debugging Documentation](https://code.visualstudio.com/docs/editor/debugging)
