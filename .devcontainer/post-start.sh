#!/bin/bash
# Auto-start script for QEMU network infrastructure
# This runs automatically when Codespace starts
#
# Services started:
# 1. TUN Bridge (waits for QEMU to be available)
# 2. HTTP Proxy (retries until ESP32 is reachable)
#
# Both services run in background with quiet mode enabled.
# Errors are logged to /workspaces/esp32-distance/temp/*_errors.log

set -e

PROJECT_DIR="/workspaces/esp32-distance"
LOG_DIR="${PROJECT_DIR}/temp"

# Ensure log directory exists
mkdir -p "${LOG_DIR}"

echo "Ensuring QEMU network infrastructure is ready..."

# Use the idempotent ensure-network-stack script
# This script checks if services are already running before starting them
"${PROJECT_DIR}/tools/ensure-network-stack.sh"

echo ""
echo "✓ Network infrastructure ready"
echo ""
echo "To access webserver:" 
echo "  1. Start QEMU: Click 'Debug' button in VS Code"
echo "  2. Open forwarded port 8080 in browser"
echo ""
echo "To check service status:"
echo "  ps aux | grep -E '(serial_tun_bridge|http_proxy)'"
echo ""
echo "To stop services:"
echo "  sudo pkill -f serial_tun_bridge"
echo "  pkill -f http_proxy"
echo ""

