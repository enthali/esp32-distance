#!/bin/bash
# Stop the ESP32 Web Flasher server

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}🛑 Stopping ESP32 Web Flasher server...${NC}"

# Kill all related processes
pkill -9 -f "flasher_server.py" 2>/dev/null && echo -e "${GREEN}✅ Killed flasher_server.py${NC}" || echo -e "${YELLOW}No flasher_server.py running${NC}"
pkill -9 -f "python3.*http.server.*8001" 2>/dev/null && echo -e "${GREEN}✅ Killed http.server on port 8001${NC}" || echo -e "${YELLOW}No http.server on port 8001${NC}"

# Use fuser to kill anything on port 8001
if command -v fuser &> /dev/null; then
    fuser -k 8001/tcp 2>/dev/null && echo -e "${GREEN}✅ Freed port 8001 (fuser)${NC}" || true
fi

# Use lsof to kill anything on port 8001
if command -v lsof &> /dev/null; then
    lsof -ti:8001 | xargs -r kill -9 2>/dev/null && echo -e "${GREEN}✅ Freed port 8001 (lsof)${NC}" || true
fi

sleep 1

# Verify port is free
if lsof -Pi :8001 -sTCP:LISTEN -t >/dev/null 2>&1; then
    echo -e "${RED}❌ Port 8001 is still in use:${NC}"
    lsof -Pi :8001 -sTCP:LISTEN
    exit 1
else
    echo -e "${GREEN}✅ Port 8001 is free${NC}"
fi
