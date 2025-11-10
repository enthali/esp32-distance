# Project Tools

This directory contains various utility scripts and tools for development, testing, and deployment of this ESP32 project.

## Directory Structure

```text
tools/
├── README.md                   # This file
├── devcontainer/               # Container Lifecycle Scripts
├── qemu/                       # QEMU Emulation
├── web-flasher/                # Web-based Flashing
├── network/                    # Network Tools
├── certificates/               # Certificate Management
├── config/                     # Configuration Tools
├── testing/                    # Test Utilities
└── system/                     # System Utilities
```

## Tool Categories

### 📦 devcontainer/

Container lifecycle management scripts for GitHub Codespaces and local dev containers.

- `on-create.sh` - Runs when container is created (one-time setup)
- `on-start.sh` - Runs when container starts (every time)
- `on-attach.sh` - Runs when attaching to container

### 🖥️ qemu/

QEMU emulation tools for testing ESP32 firmware without physical hardware.

- `run-qemu-graphics.sh` - Start QEMU with graphics and debug server
- `run-qemu-graphics-run.sh` - Start QEMU in run mode (no debug)
- `run-qemu-network.sh` - Start QEMU with network support
- `stop_qemu.sh` - Stop all running QEMU instances
- `view_uart1.sh` - View UART1 output from QEMU
- `serial_tun_bridge.py` - Bridge serial port to TUN network interface

**Usage:**

```bash
# Start QEMU with debugging
./tools/qemu/run-qemu-graphics.sh

# Run without debugging
./tools/qemu/run-qemu-graphics-run.sh

# Stop QEMU
./tools/qemu/stop_qemu.sh
```

### 🌐 web-flasher/

Web-based firmware flashing interface using ESP Web Tools.

- `web-flasher.html` - Web interface for flashing
- `flasher_server.py` - HTTP server for web flasher
- `generate-flasher-manifest.sh` - Generate manifest.json
- `start-web-flasher.sh` - Start web flasher server
- `stop-web-flasher.sh` - Stop web flasher server

**Usage:**

```bash
# Start web flasher
./tools/web-flasher/start-web-flasher.sh

# Access at http://localhost:8001 (port forwarding required)
```

### 🔌 network/

Network configuration and proxy tools for QEMU and development.

- `http_proxy.py` - HTTP proxy for QEMU network access
- `ensure-network-stack.sh` - Ensure network stack is properly configured

**Usage:**

```bash
# Start HTTP proxy for QEMU
python3 tools/network/http_proxy.py

# Ensure network stack
./tools/network/ensure-network-stack.sh
```

### 🔐 certificates/

SSL/TLS certificate generation and management tools.

- `generate_cert.py` - Generate self-signed certificates for HTTPS

**Usage:**

```bash
# Generate certificates for web server
python3 tools/certificates/generate_cert.py
```

### ⚙️ config/

Configuration file generation and management tools.

- `generate_config_factory.py` - Generate factory default configuration

**Usage:**

```bash
# Generate factory config
python3 tools/config/generate_config_factory.py
```

### 🧪 testing/

Testing utilities and validation scripts.

- `test_crypto.py` - Test cryptographic functions

**Usage:**

```bash
# Run crypto tests
python3 tools/testing/test_crypto.py
```

### 🖱️ system/

System-level utilities for desktop environment and peripherals.

- `start-vnc.sh` - Start VNC server for remote desktop
- `switch-keyboard.sh` - Switch keyboard layout

**Usage:**

```bash
# Start VNC server
./tools/system/start-vnc.sh

# Switch keyboard layout
./tools/system/switch-keyboard.sh
```

## Common Workflows

### Development with QEMU

1. Build the project: `idf.py build`
2. Start QEMU: `./tools/qemu/run-qemu-graphics.sh`
3. Debug with GDB in VS Code

### Flashing Real Hardware

1. Build the project: `idf.py build`
2. Start web flasher: `./tools/web-flasher/start-web-flasher.sh`
3. Open browser and follow instructions

### Certificate Setup

1. Generate certificates: `python3 tools/certificates/generate_cert.py`
2. Rebuild project to embed certificates
3. Flash to device

## Contributing

When adding new tools:

1. Place them in the appropriate category directory
2. Add documentation to this README
3. Include usage examples
4. Update related configuration files if needed

## See Also

- [Build Instructions](../.github/prompt-snippets/build-instructions.md)
- [Development Workflow](../.github/prompt-snippets/development.md)
- [Project Documentation](../docs/)
