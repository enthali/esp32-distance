# Build Instructions for ESP32 Projects

This document provides detailed build, flash, and debugging instructions for ESP32 projects using the devcontainer environment.

## Prerequisites

- **ESP-IDF**: Version 5.4.1 (pre-installed in devcontainer)
- **Development Environment**: GitHub Codespaces or VS Code with Dev Containers
- **Hardware** (optional): ESP32 development board connected via USB
- **QEMU** (included): For testing without physical hardware

## Environment Setup

The devcontainer automatically sets up the ESP-IDF environment. All tools are ready to use immediately after container startup.

## Build Commands

### Standard Build Process

1. **Set Target** (first time only):

   ```bash
   idf.py set-target esp32
   ```

2. **Build Project**:

   ```bash
   idf.py build
   ```

3. **Flash to Hardware** (if connected):

   ```bash
   idf.py flash monitor
   ```

4. **Run in QEMU Emulator**:

   Use the VS Code task: `Run QEMU (No Debug)` or `Start QEMU Debug Server`

### Clean Build

For a completely clean build:

```bash
idf.py fullclean
idf.py build
```

## Troubleshooting

### Build Issues & Component Structure

1. **Module Not Found Errors**:
   - Verify all custom components are properly linked in `CMakeLists.txt`
   - Check component dependencies in `main/CMakeLists.txt`

2. **Component Errors**:
   - Ensure component structure follows ESP-IDF conventions:
     - Component folder structure:
       ```text
       components/my_component/
       ├── CMakeLists.txt
       ├── include/
       │   └── my_component.h
       ├── my_component.c
       └── README.md
       ```
     - Check `CMakeLists.txt` in component directories

### Flash Issues (Hardware)

1. **Port Access Denied**:
   - In Codespaces: Hardware flashing not supported, use QEMU
   - In local devcontainer: Check USB passthrough configuration

2. **Flash Failed**:
   - Verify ESP32 is in download mode
   - Check USB cable connection
   - Try a different USB port or cable

### QEMU Issues

1. **QEMU Won't Start**:
   - Check if another QEMU instance is running: `pkill qemu`
   - Use VS Code task: `Stop QEMU` then restart

2. **Network Issues in QEMU**:
   - Verify `netif_uart_tunnel` component is enabled
   - Check QEMU network configuration in `tools/qemu/`

## Debugging

### Using VS Code

1. **QEMU Debug**: Use launch configuration `Debug in QEMU`
2. **Hardware Debug**: Use launch configuration `Debug on ESP32` (requires hardware debugger)
