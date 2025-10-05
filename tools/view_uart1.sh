#!/bin/bash
# Simple helper to view UART1 output (LED visualization)
#
# Run this in a separate terminal while QEMU is running
# to see the LED strip visualization on a separate channel.

UART1_SOCKET="/workspaces/esp32-distance/build/esp32-uart1.sock"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== UART1 LED Visualization Monitor ===${NC}"
echo -e "Reading from: ${UART1_SOCKET}"
echo -e "${YELLOW}Press Ctrl+C to stop${NC}"
echo ""

# Wait for socket to exist
WAIT_COUNT=0
while [ ! -S "${UART1_SOCKET}" ]; do
    if [ $WAIT_COUNT -eq 0 ]; then
        echo "Waiting for QEMU to start and create socket..."
    fi
    sleep 1
    WAIT_COUNT=$((WAIT_COUNT + 1))
    
    # Timeout after 30 seconds
    if [ $WAIT_COUNT -gt 30 ]; then
        echo -e "${RED}Error: Timeout waiting for UART1 socket!${NC}"
        echo -e "${YELLOW}Make sure QEMU is running: ./tools/run_qemu_dual_uart.sh${NC}"
        exit 1
    fi
done

echo -e "${GREEN}Connected to UART1!${NC}"
echo ""

# Connect to socket using netcat (nc)
nc -U ${UART1_SOCKET}
