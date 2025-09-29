# Setup Instructions

## Prerequisites

- GitHub Codespaces with ESP-IDF environment
- Chrome or Edge browser (Web Serial API support)
- ESP32 connected via USB to local machine
- Node.js installed in Codespace

## Installation

### 1. Install Node.js in Codespace

```bash
sudo apt update
sudo apt install -y nodejs npm
```

### 2. Install Bridge Server Dependencies

```bash
cd esp32-codespace-debug-bridge/bridge-server
npm install
```

### 3. Start Bridge Server

```bash
npm start
# or
node bridge.js
```

### 4. Forward Port in Codespaces

1. Open Codespaces port forwarding panel
2. Forward port `8081` with visibility "Public"
3. Note the forwarded URL: `https://<codespace>-8081.githubpreview.dev/`

### 5. Open Web Client

1. Open the forwarded URL in Chrome/Edge browser
2. Connect your ESP32 via USB to local machine
3. Click "Connect Serial & WebSocket" button
4. Select the ESP32 serial port when prompted

### 6. Start GDB Session

In Codespace terminal:

```bash
# Build and flash your ESP32 project first
idf.py build flash

# Start GDB
xtensa-esp32-elf-gdb build/your-app.elf

# In GDB prompt
(gdb) target remote localhost:3333
(gdb) monitor reset
(gdb) continue
```

## Port Configuration

- **8081**: WebSocket server (forward this port)
- **3333**: TCP server for GDB (internal, no forwarding needed)

## Troubleshooting

### Web Serial Issues
- Ensure you're using Chrome or Edge browser
- Check that ESP32 is properly connected via USB
- Try different USB cables/ports

### Connection Issues
- Verify port 8081 is forwarded in Codespaces
- Check that bridge server is running
- Ensure no other applications are using the serial port

### GDB Issues
- Make sure ESP32 firmware includes debug stub (default in ESP-IDF)
- Verify the ELF file path is correct
- Try resetting ESP32 before connecting GDB