# ESP32 Codespace Debug Bridge

A WebSerial ↔ WebSocket ↔ TCP bridge for debugging ESP32 firmware in GitHub Codespaces using VS Code and GDB.

## Problem

In GitHub Codespaces, you can't directly access serial ports (`/dev/ttyUSB0`) for ESP32 debugging with GDB. This bridge solves that by creating a connection chain:

```
ESP32 ↔ Browser WebSerial ↔ WebSocket ↔ TCP ↔ GDB (in Codespace)
```

## Architecture

- **Browser Client**: Uses Web Serial API to connect to ESP32, forwards data via WebSocket
- **Bridge Server**: Node.js server in Codespace that bridges WebSocket ↔ TCP for GDB
- **GDB**: Connects to `localhost:3333` as if it were a hardware debugger

## Quick Start

1. Start the bridge server in Codespace:
   ```bash
   cd esp32-codespace-debug-bridge/bridge-server
   npm install
   node bridge.js
   ```

2. Forward port 8081 in Codespaces UI

3. Open web client in browser:
   ```
   https://<your-codespace>-8081.githubpreview.dev/
   ```

4. Connect ESP32 via USB to your local machine, click "Connect Serial & WebSocket"

5. In Codespace terminal:
   ```bash
   xtensa-esp32-elf-gdb build/your-app.elf
   (gdb) target remote localhost:3333
   ```

## Directory Structure

- `bridge-server/` - Node.js WebSocket↔TCP bridge
- `web-client/` - Browser WebSerial interface  
- `docs/` - Documentation
- `examples/` - VS Code launch.json and GDB configurations

## Requirements

- Chrome/Edge browser (Web Serial API support)
- ESP32 connected via USB to local machine
- GitHub Codespaces with ESP-IDF environment

## Status

🚧 **Experimental** - This is a DIY solution for ESP32 debugging in cloud environments.

## License

MIT - This tool is designed to be reusable across ESP32 projects.