# Usage Guide

## Basic Workflow

1. **Start Bridge**: Run the bridge server in Codespace
2. **Connect Hardware**: Connect ESP32 via USB to local machine  
3. **Forward Port**: Forward port 8081 in Codespaces UI
4. **Open Web Client**: Access the web interface in browser
5. **Connect Serial**: Click "Connect Serial & WebSocket" 
6. **Start Debugging**: Launch GDB and connect to localhost:3333

## Web Interface

### Connection Status
- **Serial Port**: Shows ESP32 USB connection status
- **WebSocket**: Shows connection to Codespace bridge
- **Data Flow**: Displays current data transfer direction and size

### Controls
- **Connect Serial & WebSocket**: Initiates connection to ESP32 and bridge
- **Disconnect**: Closes all connections
- **Baud Rate**: Configurable serial port speed (default: 115200)

### Debug Log
- Real-time log of connection events and data transfers
- Useful for troubleshooting connection issues
- Can be cleared with "Clear Log" button

## GDB Commands

### Basic Session
```bash
# Connect to ESP32
(gdb) target remote localhost:3333

# Reset and halt ESP32
(gdb) monitor reset halt

# Set breakpoints
(gdb) break app_main
(gdb) break distance_sensor_read

# Continue execution
(gdb) continue

# Step through code
(gdb) step
(gdb) next

# Examine variables
(gdb) print my_variable
(gdb) info locals

# Examine memory
(gdb) x/16x 0x40000000

# Detach and quit
(gdb) detach
(gdb) quit
```

### ESP32-Specific Commands
```bash
# Show current CPU registers
(gdb) info registers

# Show call stack
(gdb) backtrace

# Show FreeRTOS tasks
(gdb) info threads

# Switch between tasks
(gdb) thread 2

# Monitor ESP32 system
(gdb) monitor esp32 sysview start
```

## Advanced Features

### Multiple Debug Sessions
- The bridge supports multiple GDB connections
- Each connection gets a unique client ID in logs
- Useful for debugging multiple ESP32s simultaneously

### Background Operation
- Bridge server runs continuously in background
- Web client automatically reconnects after network interruptions
- Serial connection survives browser tab switches

### Performance Monitoring
- Data flow indicators show real-time transfer rates
- Connection logs help identify bottlenecks
- Latency typically <50ms for local connections

## Best Practices

### Development Workflow
1. Keep bridge server running during development session
2. Use VS Code integrated terminal for GDB
3. Set common breakpoints in launch.json for quick access
4. Monitor debug log for connection health

### Debugging Tips
- Reset ESP32 before starting new debug session
- Use `monitor reset` instead of hardware reset when possible
- Close other serial monitors before connecting GDB
- Increase baud rate for faster data transfer if needed

### Troubleshooting
- Check web client log for connection errors
- Verify port forwarding is active and public
- Test serial connection independently first
- Use different USB port if connection is unstable