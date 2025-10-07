# Build Instructions for ESP32 Distance Sensor Project

This document provides detailed build, flash, and debugging instructions for the ESP32 Distance Sensor project.

## Prerequisites

- **ESP-IDF**: Version 5.4.1 or compatible
- **Hardware**: ESP32 development board connected via USB
- **Operating System**: Windows (PowerShell), Linux, or macOS
- **Git**: For version control and branch management

## Environment Setup

### Windows with PowerShell

Ensure ESP-IDF is properly installed and configured:

```powershell
# Verify ESP-IDF installation
Get-Command idf.py -ErrorAction SilentlyContinue
```

If `idf.py` is not found, you need to set up the ESP-IDF environment first.

## Build Commands

### Standard Build Process

1. **Set Target**:

   ```bash
   idf.py set-target esp32
   ```

2. **Build Project**:

   ```bash
   idf.py build
   ```

3. **Flash and Monitor**:

   ```bash
   idf.py flash monitor
   ```

### Windows-Specific Complete Command

For Windows users working with PowerShell, use this complete command that handles environment setup automatically:

```powershell
cmd /c "cd /D c:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py flash monitor"
```

**Command Breakdown**:

- `cmd /c`: Executes command in Windows Command Prompt
- `cd /D c:\workspace\ESP32_Projects\distance`: Changes to project directory
- `C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat`: Sets up ESP-IDF environment
- `idf.py flash monitor`: Flashes firmware and starts serial monitor

### Alternative Commands

**Build Only**:

```powershell
cmd /c "cd /D c:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py build"
```

**Flash Only**:

```powershell
cmd /c "cd /D c:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py flash"
```

**Monitor Only**:

```powershell
cmd /c "cd /D c:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py monitor"
```

## Troubleshooting

### Build Issues

1. **Module Not Found Errors**:
   - Verify all custom components are properly linked in `CMakeLists.txt`
   - Check component dependencies in `main/CMakeLists.txt`

2. **Environment Not Set**:
   - Ensure ESP-IDF export script path is correct
   - Verify ESP-IDF installation location

3. **Permission Denied**:
   - Close any open serial monitor applications
   - Check USB connection and driver installation

### Flash Issues

1. **Port Access Denied**:
   - Close other applications using the serial port
   - Disconnect and reconnect USB cable
   - Check device manager for proper COM port assignment

2. **Flash Failed**:
   - Verify ESP32 is in download mode
   - Check USB cable connection
   - Try different USB port

### Monitor Issues

1. **No Output**:
   - Verify baud rate (default: 115200)
   - Check if device is properly reset after flashing
   - Ensure correct COM port selection

2. **Garbled Output**:
   - Verify baud rate settings
   - Check for electromagnetic interference

## Development Workflow

### Git Branch Management

When working on build fixes or new features:

```bash
# Create feature branch
git checkout -b fix/build-issues

# Make changes and test
cmd /c "cd /D c:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py build"

# Commit changes
git add .
git commit -m "Fix: Resolve build compilation errors"

# Push and create PR
git push origin fix/build-issues
```

### Clean Build

For a completely clean build:

```bash
idf.py fullclean
idf.py build
```

## Project-Specific Notes

### Custom Components

This project includes custom components in the `components/` directory:

- `distance_sensor/`: HC-SR04 ultrasonic sensor interface
- `led_controller/`: WS2812 LED strip control

### Main Application Modules

Located in `main/` directory:

- `wifi_manager.c`: WiFi connectivity and captive portal
- `web_server.c`: HTTP server implementation
- `dns_server.c`: DNS server for captive portal
- `display_logic.c`: Distance-to-LED mapping logic

### Build Configuration

#### Flash Memory Configuration

This project is configured for ESP32 modules with **4MB flash memory**:

- **Flash Size**: 4MB (CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y)
- **Partition Table**: Single App Large (maximizes application space)
- **Free Flash**: ~41% available for application growth
- **HTTPS Ready**: Sufficient space for SSL/TLS certificates and HTTPS implementation

**Memory Usage Summary** (after optimization):

- Used flash: ~59% (vs 86% with 2MB config)
- Free flash: ~41% (vs 14% with 2MB config)
- IRAM usage: ~76% (sufficient headroom)
- DRAM usage: ~18% (low utilization, good for buffers)

#### Configuration Verification

To verify flash configuration:

```bash
idf.py size
```

To modify flash settings:

```bash
idf.py menuconfig
# Navigate to: Serial flasher config > Flash size
# Navigate to: Partition Table > Partition Table
```

#### Key Configuration Options

Key configuration options in `sdkconfig`:

- WiFi stack configuration
- FreeRTOS task stack sizes
- LED strip timing parameters
- Serial monitor baud rate
- **Flash size and partition table** (optimized for 4MB modules)

## Serial Monitor Commands

When monitoring device output:

- **Ctrl+]**: Exit monitor
- **Ctrl+T, Ctrl+R**: Reset device
- **Ctrl+T, Ctrl+H**: Show help

## Performance Testing

To verify system performance after build:

1. **Monitor startup sequence**: Check for proper WiFi initialization
2. **Test distance measurement**: Verify LED strip responds to sensor input
3. **Web interface**: Access captive portal or web server
4. **Memory usage**: Monitor heap and stack usage in logs

## Version Information

- **ESP-IDF**: v5.4.1
- **Target**: ESP32
- **Toolchain**: xtensa-esp32-elf
- **Project Version**: Check `CMakeLists.txt` for current version
