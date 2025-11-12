#!/bin/bash
# Generate ESP Web Tools manifest.json from ESP-IDF build artifacts
# This script creates a manifest that the web flasher can use to flash firmware

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$TOOLS_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
OUTPUT_FILE="$BUILD_DIR/flasher-manifest.json"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Error: Build directory not found: $BUILD_DIR"
    echo "Please run 'idf.py build' first"
    exit 1
fi

# Extract project name from CMakeLists.txt
PROJECT_NAME=$(grep -oP 'set\(PROJECT_NAME\s+"?\K[^")]+' "$PROJECT_DIR/CMakeLists.txt" | head -1 | tr -d ' ')
if [ -z "$PROJECT_NAME" ]; then
    PROJECT_NAME="esp32-distance"
fi

# Get version from VERSION file
if [ -f "$PROJECT_DIR/VERSION" ]; then
    VERSION=$(cat "$PROJECT_DIR/VERSION" | tr -d '[:space:]')
else
    VERSION="1.0.0"
fi

# Get flash addresses from build artifacts (in decimal for ESP Web Tools)
BOOTLOADER_OFFSET=4096        # 0x1000
PARTITION_TABLE_OFFSET=32768  # 0x8000
APP_OFFSET=65536              # 0x10000

# Verify required files exist
BOOTLOADER_BIN="$BUILD_DIR/bootloader/bootloader.bin"
PARTITION_TABLE_BIN="$BUILD_DIR/partition_table/partition-table.bin"
APP_BIN="$BUILD_DIR/${PROJECT_NAME}.bin"

if [ ! -f "$BOOTLOADER_BIN" ]; then
    echo "❌ Error: Bootloader binary not found: $BOOTLOADER_BIN"
    exit 1
fi

if [ ! -f "$PARTITION_TABLE_BIN" ]; then
    echo "❌ Error: Partition table binary not found: $PARTITION_TABLE_BIN"
    exit 1
fi

if [ ! -f "$APP_BIN" ]; then
    echo "❌ Error: Application binary not found: $APP_BIN"
    exit 1
fi

# Generate manifest.json with paths relative to manifest location
# Manifest is in build/, so binaries are in same directory and subdirectories
cat > "$OUTPUT_FILE" << EOF
{
  "name": "${PROJECT_NAME}",
  "version": "${VERSION}",
  "home_assistant_domain": "esphome",
  "new_install_prompt_erase": true,
  "builds": [
    {
      "chipFamily": "ESP32",
      "parts": [
        {
          "path": "bootloader/bootloader.bin",
          "offset": ${BOOTLOADER_OFFSET}
        },
        {
          "path": "partition_table/partition-table.bin",
          "offset": ${PARTITION_TABLE_OFFSET}
        },
        {
          "path": "${PROJECT_NAME}.bin",
          "offset": ${APP_OFFSET}
        }
      ]
    }
  ]
}
EOF

echo "✅ Manifest generated successfully: $OUTPUT_FILE"
echo ""
echo "📦 Firmware files:"
echo "  - Bootloader:      $(basename "$BOOTLOADER_BIN") @ $BOOTLOADER_OFFSET"
echo "  - Partition Table: $(basename "$PARTITION_TABLE_BIN") @ $PARTITION_TABLE_OFFSET"
echo "  - Application:     $(basename "$APP_BIN") @ $APP_OFFSET"
echo ""
echo "🚀 To use the web flasher:"
echo "  1. Start the web server: ./tools/start-web-flasher.sh"
echo "  2. Forward port 8001 in GitHub Codespaces"
echo "  3. Open the forwarded URL in your browser"
echo "  4. Click 'Connect and Flash ESP32'"
