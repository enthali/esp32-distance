# Development Container Setup

This project uses containerized development with VS Code Dev Containers and GitHub Codespaces for consistent ESP-IDF development environment.

## Development Options

### Recommended: GitHub Codespaces (Cloud)

- No local setup required - works entirely in browser
- Pre-configured ESP-IDF v5.4.1 environment ready instantly
- USB device access through browser serial interface

### Alternative: Local Dev Container  

- Uses Docker container with ESP-IDF v5.4.1 pre-installed  
- Requires Docker Desktop and VS Code with Dev Containers extension
- Identical environment across all team members and platforms

## Using the Dev Container

### In GitHub Codespaces

1. Go to your GitHub repository
2. Click "Code" → "Codespaces" → "Create codespace"
3. Container will automatically build with ESP-IDF ready
4. Extensions (ESP-IDF, ESP-IDF Web) will be pre-installed

### Local Development with Docker

1. Ensure Docker Desktop is running
2. Open the project in VS Code
3. VS Code will prompt: "Reopen in Container" → Click Yes
4. Container builds automatically with ESP-IDF environment

## Container Features

- **ESP-IDF v5.4.1**: Matches your local Windows version
- **Pre-configured VS Code**: ESP-IDF extension settings ready
- **Web Serial Support**: ESP-IDF Web extension for Codespaces flashing/monitoring  
- **Port forwarding**: HTTP server (80, 443) and development ports
- **Privileged mode**: USB device access for flashing (local Docker only)

## Build Commands in Container

```bash
# Standard ESP-IDF commands work directly
idf.py build
idf.py flash monitor
idf.py menuconfig

# Container has ESP-IDF environment pre-loaded
```

## Hardware Flashing

- **Local Docker**: Hardware connected to Windows is accessible in container
- **Codespaces**: Use ESP-IDF Web extension for WebSerial/WebUSB flashing
- **Mixed workflow**: Build in container, flash on Windows if needed

### Windows USB Device Setup (Local Dev Container Only)

For Windows users running the dev container locally (not needed for Codespaces):

**Quick Setup**: Run `tools/attach-esp32.ps1` as Administrator - script automatically finds and attaches your ESP32.

**Manual Setup** (if script doesn't work):

1. Install usbipd-win: `winget install usbipd`
2. Find device: `usbipd list` (look for USB-SERIAL CH340)
3. Bind device: `usbipd bind --busid <BUSID>`
4. Attach to container: `usbipd attach --wsl --busid <BUSID>`

**Note**: GitHub Codespaces handles USB through browser serial interface - no manual setup needed.

## Benefits

- **Consistent environments** across Windows, macOS, Linux, Codespaces
- **No ESP-IDF setup** required for new team members
- **Isolated dependencies** - container changes don't affect host system
- **Easy onboarding** - clone repo, open in VS Code, start coding
- **Your Windows setup unchanged** - existing workflow still works
