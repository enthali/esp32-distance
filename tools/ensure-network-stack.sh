#!/bin/bash
# Ensure network stack (TUN + HTTP Proxy) is running
# This script is idempotent - can be called multiple times safely

set -e

PROJECT_DIR="/workspaces/esp32-distance"
TUN_SCRIPT="${PROJECT_DIR}/tools/serial_tun_bridge.py"
PROXY_SCRIPT="${PROJECT_DIR}/tools/http_proxy.py"
TUN_LOG="${PROJECT_DIR}/temp/tun_errors.log"
PROXY_LOG="${PROJECT_DIR}/temp/proxy_errors.log"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Checking network stack...${NC}"

# Check if TUN bridge is running
if pgrep -f "serial_tun_bridge.py" > /dev/null; then
    echo -e "${GREEN}✓${NC} TUN bridge already running"
else
    echo -e "${YELLOW}Starting TUN bridge...${NC}"
    sudo "${TUN_SCRIPT}" --quiet 2>"${TUN_LOG}" &
    sleep 1
    if pgrep -f "serial_tun_bridge.py" > /dev/null; then
        echo -e "${GREEN}✓${NC} TUN bridge started"
    else
        echo -e "${YELLOW}⚠${NC} TUN bridge start failed (check ${TUN_LOG})"
    fi
fi

# Check if HTTP proxy is running
if pgrep -f "http_proxy.py" > /dev/null; then
    echo -e "${GREEN}✓${NC} HTTP proxy already running"
else
    echo -e "${YELLOW}Starting HTTP proxy...${NC}"
    python3 "${PROXY_SCRIPT}" --quiet 2>"${PROXY_LOG}" &
    sleep 0.5
    if pgrep -f "http_proxy.py" > /dev/null; then
        echo -e "${GREEN}✓${NC} HTTP proxy started"
    else
        echo -e "${YELLOW}⚠${NC} HTTP proxy start failed (check ${PROXY_LOG})"
    fi
fi

echo -e "${BLUE}Network stack ready${NC}"
