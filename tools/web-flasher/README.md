# ESP32 Web Flasher

Flash your ESP32 directly from your browser using the Web Serial API! No Python, no esptool, no command-line hassle.

## 🎯 Quick Start

```bash
# 1. Build your firmware
idf.py build

# 2. Start web flasher server
./tools/web-flasher/start-web-flasher.sh

# 3. Forward port 8001 in VS Code (Ports tab)

# 4. Open the forwarded URL in Chrome/Edge/Opera

# 5. Click "Connect and Flash ESP32"
```

## 🌟 Features

- ✅ **Browser-based flashing** - No installation required
- ✅ **One-click operation** - Flash bootloader, partition table, and app
- ✅ **Serial monitor** - View ESP32 output directly in browser
- ✅ **Progress tracking** - Real-time flashing progress
- ✅ **GitHub Codespaces compatible** - Works perfectly in remote environments

## 📋 Requirements

### Browser Support

- **Chrome** 89+
- **Edge** 89+
- **Opera** 75+
- ❌ Firefox (Web Serial API not supported yet)
- ❌ Safari (Web Serial API not supported yet)

### Hardware

- ESP32 connected via USB to your **local computer** (not the container)
- USB cable with data lines (not charge-only)

## 🚀 Usage

### Step 1: Build Firmware

```bash
cd /workspaces/esp32-distance  # Or your project directory
idf.py build
```

### Step 2: Start Web Server

```bash
./tools/start-web-flasher.sh
```

This will:

- Generate `manifest.json` with flash offsets
- Start HTTP server on port 8001
- Serve the web flasher interface

### Step 3: Port Forwarding

In VS Code (GitHub Codespaces):

1. Open the **Ports** tab (next to Terminal)
2. Click **Forward a Port**
3. Enter `8001`
4. Set visibility to **Public** if accessing from different network
5. Click on the forwarded URL

### Step 4: Connect ESP32

1. Connect ESP32 via USB to your **local computer**
2. In the browser, click **"Connect and Flash ESP32"**
3. Select your ESP32's serial port from the dropdown
4. Click **Install** to flash

### Step 5: Monitor

After flashing completes, the browser will show:

- ✅ Installation successful
- 📡 Serial console output
- 🔄 Option to reconnect

## 🔧 How It Works

### Architecture

```text
┌─────────────────┐
│  GitHub         │
│  Codespaces     │
│  Container      │
│                 │
│  ┌───────────┐  │
│  │ HTTP      │  │  Port 8001 Forwarded
│  │ Server    │  │◄─────────────────────┐
│  └───────────┘  │                      │
│  ┌───────────┐  │                      │
│  │ Build     │  │                      │
│  │ Artifacts │  │                      │
│  └───────────┘  │                      │
└─────────────────┘                      │
                                         │
                           ┌─────────────┴──────────────┐
                           │  Your Local Computer       │
                           │  ┌──────────────────────┐  │
                           │  │  Chrome Browser      │  │
                           │  │  (Web Serial API)    │  │
                           │  └──────────┬───────────┘  │
                           │             │              │
                           │             │ USB          │
                           │             ↓              │
                           │      ┌────────────┐        │
                           │      │   ESP32    │        │
                           │      └────────────┘        │
                           └────────────────────────────┘
```

### Technology Stack

- **[ESP Web Tools](https://esphome.github.io/esp-web-tools/)**: Official Espressif web-based flashing library
- **Web Serial API**: Browser API for serial port access
- **esptool-js**: JavaScript port of esptool.py
- **Python HTTP Server**: Simple static file server

### Manifest Format

The `manifest.json` tells the web flasher what to flash and where:

```json
{
  "name": "your-project-name",
  "version": "1.0.0",
  "builds": [
    {
      "chipFamily": "ESP32",
      "parts": [
        {
          "path": "../build/bootloader/bootloader.bin",
          "offset": 4096
        },
        {
          "path": "../build/partition_table/partition-table.bin",
          "offset": 32768
        },
        {
          "path": "../build/your-project-name.bin",
          "offset": 65536
        }
      ]
    }
  ]
}
```

## 🐛 Troubleshooting

### Port Not Appearing in Browser

**Problem**: Serial port dropdown is empty

**Solutions**:

- ✅ Use Chrome, Edge, or Opera (not Firefox/Safari)
- ✅ Connect ESP32 to **local computer**, not container
- ✅ Check USB cable supports data (not charge-only)
- ✅ Install CH340/CP2102 drivers if needed
- ✅ Close other apps using serial port (Arduino IDE, PuTTY, etc.)

### Connection Refused

**Problem**: Can't access <http://localhost:8001>

**Solutions**:

- ✅ Verify web server is running (`./tools/web-flasher/start-web-flasher.sh`)
- ✅ Check port 8001 is forwarded in VS Code Ports tab
- ✅ Try using the forwarded URL instead of localhost
- ✅ Check firewall settings

### Flash Failed

**Problem**: Flashing fails or times out

**Solutions**:

- ✅ Hold **BOOT** button on ESP32 while connecting
- ✅ Press **RESET** button before flashing
- ✅ Try different USB port
- ✅ Verify firmware built successfully (`idf.py build`)
- ✅ Check `manifest.json` has correct paths

### Build Not Found

**Problem**: Error: "Build directory not found"

**Solutions**:

- ✅ Run `idf.py build` first
- ✅ Check `build/` directory exists
- ✅ Verify bootloader.bin and partition-table.bin exist

### Permission Denied (Linux/Mac)

**Problem**: Can't access serial port

**Solutions**:

```bash
# Add user to dialout group (Linux)
sudo usermod -a -G dialout $USER

# Change port permissions (temporary)
sudo chmod 666 /dev/ttyUSB0
```

## 📚 Additional Resources

- [ESP Web Tools Documentation](https://esphome.github.io/esp-web-tools/)
- [Web Serial API Specification](https://wicg.github.io/serial/)
- [ESP-IDF Flashing Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-py.html#flash)
- [esptool.py Documentation](https://docs.espressif.com/projects/esptool/en/latest/)

## 🔐 Security Considerations

### Web Serial API Access

- The Web Serial API requires **user interaction** (button click)
- Ports are **not accessible** without explicit user permission
- Each session requires **re-authorization**
- No background access to serial ports

### HTTPS Requirements

- Web Serial API works on:
  - ✅ `http://localhost:*`
  - ✅ `https://*` (any HTTPS site)
  - ❌ `http://*` (non-localhost HTTP)

GitHub Codespaces port forwarding provides HTTPS automatically! 🎉

## 🎓 Advanced Usage

### Customizing the Manifest

Edit `tools/generate-flasher-manifest.sh` to:

- Change flash offsets
- Add additional partitions (NVS, SPIFFS, etc.)
- Support multiple chip families (ESP32-S2, ESP32-C3, etc.)

### Adding Serial Console Features

The ESP Web Tools library supports:

- **Baud rate selection**
- **Serial output logging**
- **Command sending**
- **Download logs as text**

Customize `tools/web-flasher.html` to enable these features.

### Automation

Generate manifest automatically after build:

```bash
# Add to CMakeLists.txt
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/tools/generate-flasher-manifest.sh
)
```

## 💡 Tips & Tricks

### Quick Rebuild and Flash

```bash
# One-liner to rebuild and update manifest
idf.py build && ./tools/generate-flasher-manifest.sh
```

### Keep Server Running

```bash
# Run server in background
nohup ./tools/web-flasher/start-web-flasher.sh &

# Stop server
pkill -f flasher_server.py
# or use the stop script
./tools/web-flasher/stop-web-flasher.sh
```

### Testing Without Hardware

Use QEMU to test firmware before flashing:

```bash
## Testing in QEMU

You can test the web interface without hardware using QEMU:

```bash
# Start QEMU
./tools/qemu/run-qemu-graphics.sh
```

## 📝 Files Created

```text
tools/
├── web-flasher.html              # Web interface
├── generate-flasher-manifest.sh  # Generate manifest.json
├── start-web-flasher.sh          # Start HTTP server
├── manifest.json                 # Generated flash configuration
└── WEB-FLASHER-README.md         # This file
```

## 🤝 Contributing

Improvements welcome! Some ideas:

- [ ] Add ESP32-S2/S3/C3 support
- [ ] Support OTA updates
- [ ] Add firmware version checking
- [ ] Integrate with CI/CD
- [ ] Add dark mode to web interface

## 📄 License

Same as parent project (see root LICENSE file)

## 🎉 Credits

- **ESP Web Tools**: [ESPHome Team](https://github.com/esphome/esp-web-tools)
- **esptool-js**: [Espressif Systems](https://github.com/espressif/esptool-js)
- **Web Serial API**: [W3C WICG](https://wicg.github.io/serial/)
