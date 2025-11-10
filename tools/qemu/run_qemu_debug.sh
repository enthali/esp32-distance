#!/bin/bash
# Start QEMU with graphics and wait for GDB debugger (Debug Mode)
# Usage: ./tools/qemu/run_qemu_debug.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
UART0_SOCKET="${PROJECT_DIR}/temp/esp32-uart0.sock"
UART1_SOCKET="${PROJECT_DIR}/temp/esp32-uart1.sock"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

if [ -z "$DISPLAY" ]; then
    echo -e "${YELLOW}Setting DISPLAY=:1 for GUI...${NC}"
    export DISPLAY=:1
fi

"${PROJECT_DIR}/tools/qemu/network/ensure-network-stack.sh"
echo -e "${YELLOW}Starting QEMU in Debug Mode (graphics, waits for GDB)...${NC}"

cd "${PROJECT_DIR}"

# Start idf.py qemu in background (will build first if needed)
idf.py qemu -d -g --qemu-extra-args="-serial unix:${UART0_SOCKET},server,nowait -serial unix:${UART1_SOCKET},server,nowait" &
QEMU_PID=$!

echo -e "${YELLOW}QEMU build/start process running with PID: ${QEMU_PID}${NC}"

# Wait for ELF file to exist (created by build process)
ELF_FILE="${PROJECT_DIR}/build/esp32-template.elf"
echo -e "${YELLOW}Waiting for build to complete...${NC}"
while [ ! -f "${ELF_FILE}" ]; do
    sleep 0.5
done
echo -e "${GREEN}✓ Build complete: ${ELF_FILE}${NC}"

# Wait for UART sockets (QEMU has started)
echo -e "${YELLOW}Waiting for QEMU to start...${NC}"
while [ ! -S "${UART0_SOCKET}" ] || [ ! -S "${UART1_SOCKET}" ]; do
    sleep 0.5
done
echo -e "${GREEN}✓ QEMU UART0 socket ready: ${UART0_SOCKET}${NC}"
echo -e "${GREEN}✓ QEMU UART1 socket ready: ${UART1_SOCKET}${NC}"

sleep 1
echo -e "${YELLOW}QEMU running with PID: ${QEMU_PID}${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop QEMU${NC}"
wait $QEMU_PID
