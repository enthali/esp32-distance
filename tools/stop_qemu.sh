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
UART1_SOCKET="/workspaces/esp32-distance/build/esp32-uart1.sock"
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

echo -e "${GREEN}Cleanup complete.${NC}"
