# Development Environment

This section covers all aspects of developing the ESP32 Distance Sensor project, from setting up your environment to debugging and testing.

## Quick Links

- **[Dev Container Setup](devcontainer.md)** - GitHub Codespaces and local Docker development
- **[QEMU Emulator](qemu-emulator.md)** - Running the project in QEMU with network support
- **[Debugging](debugging.md)** - GDB debugging with QEMU and VS Code
- **[Network Internals](qemu-network-internals.md)** - Deep dive into QEMU IP tunnel implementation

## Getting Started

### Recommended: GitHub Codespaces (Cloud)
The fastest way to start developing:

1. Open the repository in GitHub Codespaces
2. Wait for container to build (first time only)
3. Run QEMU: `./tools/run-qemu-network.sh`
4. Start debugging with F5 or access the web interface

### Alternative: Local Dev Container
For offline development or custom hardware:

1. Install Docker Desktop
2. Open project in VS Code
3. "Reopen in Container" when prompted
4. Connect hardware via USB (see [Dev Container Setup](devcontainer.md))

## Development Workflow

```bash
# Build the project
idf.py build

# Flash to hardware (hardware mode)
idf.py -p /dev/ttyUSB0 flash monitor

# Run in emulator (emulator mode)
./tools/run-qemu-network.sh

# Debug in emulator
# Press F5 in VS Code or use Debug panel
```

## Key Features

- **Consistent Environment**: Same toolchain everywhere (Codespaces, Docker, CI/CD)
- **ESP-IDF v5.4.1**: Latest stable version pre-configured
- **QEMU Support**: Test without hardware using full network emulation
- **GDB Debugging**: Full breakpoint debugging in emulator
- **Web Interface**: Access emulated web server via browser

## Next Steps

- New to dev containers? Start with [Dev Container Setup](devcontainer.md)
- Want to debug? Check out [Debugging Guide](debugging.md)
- Curious about QEMU networking? See [QEMU Emulator](qemu-emulator.md)
- Need technical details? Read [Network Internals](qemu-network-internals.md)

## Known Issues

Unfortunately ESP-IDF-WEB doesn't automatically install. please search the extension in the marketplace and install it.
