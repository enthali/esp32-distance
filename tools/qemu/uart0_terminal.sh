#!/bin/bash
# Interactive UART0 Terminal for ESP32 QEMU
#
# Connects to ESP32's UART0 socket for bidirectional communication.
# Use this to interact with the ESP32 console (read logs, send commands).
#
# Usage: ./uart0_terminal.sh
# Exit: Ctrl+C to disconnect

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Determine project directory (two levels up from tools/qemu/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

UART0_SOCKET="${PROJECT_DIR}/temp/esp32-uart0.sock"

echo -e "${GREEN}ESP32 UART0 Interactive Terminal${NC}"
echo -e "${YELLOW}Waiting for QEMU to start...${NC}"

# Forever loop to reconnect after QEMU restarts
while true; do
    # Wait for socket to appear
    while [ ! -S "${UART0_SOCKET}" ]; do
        sleep 1
    done
    
    echo -e "${GREEN}Connected to UART0${NC}"
    echo -e "${YELLOW}(Press Ctrl+C to exit, or wait for reconnect after QEMU restart)${NC}"
    echo ""
    
    # Connect to UART0 socket (bidirectional with netcat)
    nc -U "${UART0_SOCKET}"
    
    # If nc exits, wait before reconnecting
    echo -e "${YELLOW}Connection lost. Reconnecting...${NC}"
    sleep 1
done
