#!/bin/bash
# QEMU ESP32 Emulator with Network Tunnel Support
# 
# This script uses ESP-IDF's built-in QEMU functionality (idf.py qemu)
# which automatically creates the merged flash image and efuse binary.
#
# Features:
# - Automatic flash image generation (qemu_flash.bin)
# - GDB debug support (waits for debugger connection)
# - Dual UART configuration:
#   * UART0 (tcp:5555): Main console for logs/monitor
#   * UART1 (tcp:5556): Network tunnel for IP connectivity
#
# The TUN bridge and HTTP proxy should be started separately
# (they can start before QEMU and will connect automatically)

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

PROJECT_DIR="/workspaces/esp32-distance"
NETWORK_UART_PORT=5556

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ESP32 QEMU - Network Development Mode${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Build project first (delta build is fast, ensures latest code)
echo -e "${YELLOW}Building project...${NC}"
cd "${PROJECT_DIR}"
idf.py build || {
    echo -e "${RED}Build failed!${NC}"
    exit 1
}
echo ""

# Ensure network stack (TUN bridge + HTTP proxy) is running
echo -e "${YELLOW}Checking network infrastructure...${NC}"
"${PROJECT_DIR}/tools/ensure-network-stack.sh"
echo ""

echo -e "${YELLOW}Starting QEMU via ESP-IDF (with automatic flash image generation)...${NC}"
echo ""
echo -e "${GREEN}Network Configuration:${NC}"
echo -e "  UART0 Port: 5555 (Console/Monitor)"
echo -e "  UART1 Port: ${NETWORK_UART_PORT} (IP tunnel)"
echo -e "  ESP32 IP:   192.168.100.2"
echo -e "  Gateway:    192.168.100.1 (TUN device)"
echo ""
echo -e "${YELLOW}Note:${NC} QEMU will wait for GDB to connect"
echo -e "${YELLOW}VS Code debugger will connect automatically${NC}"
echo ""
echo -e "${BLUE}========================================${NC}"
echo ""

# Run QEMU via ESP-IDF with:
# - GDB debug support (-d flag makes it wait for debugger)
# - Two serial ports:
#   * UART0 (5555): Main console for logs/monitor
#   * UART1 (5556): Network tunnel for IP connectivity
# - ESP-IDF handles flash image creation automatically

# Start QEMU in background to capture output
idf.py qemu -d \
    --qemu-extra-args="-serial tcp::5555,server,nowait -serial tcp::${NETWORK_UART_PORT},server,nowait -nographic" &

QEMU_PID=$!

# Wait for QEMU to open GDB port (indicating it's ready)
echo -e "${YELLOW}Waiting for QEMU to start...${NC}"
for i in {1..30}; do
    if nc -z localhost 3333 2>/dev/null; then
        echo -e "${GREEN}✓ QEMU is ready and waiting for GDB connection${NC}"
        break
    fi
    sleep 0.5
done

# Wait for QEMU process to finish
wait $QEMU_PID
