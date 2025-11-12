#!/bin/bash
# Stop QEMU ESP32 Emulator
#
# Kills any running QEMU ESP32 instances and cleans up sockets.
# Based on ESP-IDF's cleanup mechanism.

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Determine project directory (two levels up from tools/qemu/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

echo -e "${YELLOW}Stopping QEMU ESP32...${NC}"

# Find and kill QEMU processes
QEMU_PIDS=$(pgrep -f "qemu-system-xtensa.*esp32" || true)

if [ -z "$QEMU_PIDS" ]; then
    echo -e "${GREEN}No QEMU processes running.${NC}"
else
    echo -e "Found QEMU processes: ${QEMU_PIDS}"
    
    # Terminate gracefully first
    for pid in $QEMU_PIDS; do
        echo -n "Terminating QEMU (PID: $pid)... "
        kill $pid 2>/dev/null && echo -e "${GREEN}✓${NC}" || echo -e "${RED}failed${NC}"
    done
    
    # Wait a moment
    sleep 1
    
    # Force kill if still running
    QEMU_PIDS=$(pgrep -f "qemu-system-xtensa.*esp32" || true)
    if [ ! -z "$QEMU_PIDS" ]; then
        echo -e "${YELLOW}Force killing remaining processes...${NC}"
        for pid in $QEMU_PIDS; do
            kill -9 $pid 2>/dev/null || true
        done
    fi
    
    echo -e "${GREEN}QEMU stopped.${NC}"
fi

# Cleanup sockets
UART1_SOCKET="${PROJECT_DIR}/build/esp32-uart1.sock"
if [ -S "$UART1_SOCKET" ]; then
    rm -f "$UART1_SOCKET"
    echo -e "${GREEN}Removed UART1 socket.${NC}"
fi

# Kill any idf_monitor processes
MONITOR_PIDS=$(pgrep -f "idf_monitor" || true)
if [ ! -z "$MONITOR_PIDS" ]; then
    echo -e "${YELLOW}Stopping idf_monitor...${NC}"
    kill $MONITOR_PIDS 2>/dev/null || true
    echo -e "${GREEN}idf_monitor stopped.${NC}"
fi

# Kill network bridge processes (may need sudo since it was started with sudo)
BRIDGE_PIDS=$(pgrep -f "serial_tun_bridge.py" || true)
if [ ! -z "$BRIDGE_PIDS" ]; then
    echo -e "${YELLOW}Stopping serial_tun_bridge...${NC}"
    sudo kill $BRIDGE_PIDS 2>/dev/null || kill $BRIDGE_PIDS 2>/dev/null || true
    sleep 0.5
    # Force kill if still running
    BRIDGE_PIDS=$(pgrep -f "serial_tun_bridge.py" || true)
    if [ ! -z "$BRIDGE_PIDS" ]; then
        sudo kill -9 $BRIDGE_PIDS 2>/dev/null || true
    fi
    echo -e "${GREEN}serial_tun_bridge stopped.${NC}"
fi

# Kill HTTP proxy processes
PROXY_PIDS=$(pgrep -f "http_proxy.py" || true)
if [ ! -z "$PROXY_PIDS" ]; then
    echo -e "${YELLOW}Stopping http_proxy...${NC}"
    kill $PROXY_PIDS 2>/dev/null || true
    echo -e "${GREEN}http_proxy stopped.${NC}"
fi

echo -e "${GREEN}Cleanup complete.${NC}"
