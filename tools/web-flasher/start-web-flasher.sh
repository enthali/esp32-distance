#!/bin/bash
# Start HTTP server for ESP32 Web Flasher
# Serves the web-flasher.html and provides access to build artifacts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$TOOLS_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║         ESP32 Template Web Flasher Server                 ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Check if build exists and has required files
BUILD_NEEDED=false
if [ ! -d "$PROJECT_DIR/build" ]; then
    BUILD_NEEDED=true
    echo -e "${YELLOW}⚠️  No build directory found${NC}"
else
    # Check for required build artifacts
    if [ ! -f "$PROJECT_DIR/build/bootloader/bootloader.bin" ] || \
       [ ! -f "$PROJECT_DIR/build/partition_table/partition-table.bin" ]; then
        BUILD_NEEDED=true
        echo -e "${YELLOW}⚠️  Build artifacts incomplete${NC}"
    fi
    
    # Check if app binary exists (extract project name first)
    PROJECT_NAME=$(grep -oP 'set\(PROJECT_NAME\s+"?\K[^")]+' "$PROJECT_DIR/CMakeLists.txt" | head -1 | tr -d ' ')
    if [ -z "$PROJECT_NAME" ]; then
        PROJECT_NAME="esp32-template"
    fi
    
    if [ ! -f "$PROJECT_DIR/build/${PROJECT_NAME}.bin" ]; then
        BUILD_NEEDED=true
        echo -e "${YELLOW}⚠️  Application binary not found${NC}"
    fi
fi

# Build if necessary
if [ "$BUILD_NEEDED" = true ]; then
    echo -e "${BLUE}🔨 Building firmware...${NC}"
    echo ""
    cd "$PROJECT_DIR"
    idf.py build
    BUILD_EXIT=$?
    
    if [ $BUILD_EXIT -ne 0 ]; then
        echo ""
        echo -e "${RED}❌ Build failed with exit code $BUILD_EXIT${NC}"
        echo -e "${YELLOW}Please fix build errors before starting web flasher${NC}"
        exit $BUILD_EXIT
    fi
    
    echo ""
    echo -e "${GREEN}✅ Build completed successfully${NC}"
    echo ""
else
    echo -e "${GREEN}✅ Build artifacts found${NC}"
fi

# Generate manifest
echo -e "${YELLOW}📝 Generating flasher manifest...${NC}"
"$SCRIPT_DIR/generate-flasher-manifest.sh"
echo ""
echo ""

# Function to kill processes on port 8001
kill_port_8001() {
    # Try multiple methods to free the port
    pkill -9 -f "flasher_server.py" 2>/dev/null || true
    pkill -9 -f "python3.*http.server.*8001" 2>/dev/null || true
    
    # Use fuser if available
    if command -v fuser &> /dev/null; then
        fuser -k 8001/tcp 2>/dev/null || true
    fi
    
    # Use lsof if available
    if command -v lsof &> /dev/null; then
        lsof -ti:8001 | xargs -r kill -9 2>/dev/null || true
    fi
    
    sleep 1
}

# Check if port 8001 is already in use
if lsof -Pi :8001 -sTCP:LISTEN -t >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠️  Port 8001 is already in use${NC}"
    echo -e "${YELLOW}Stopping existing server...${NC}"
    kill_port_8001
    
    # Verify port is free
    if lsof -Pi :8001 -sTCP:LISTEN -t >/dev/null 2>&1; then
        echo -e "${RED}❌ Failed to free port 8001${NC}"
        echo -e "${YELLOW}Please manually kill the process:${NC}"
        lsof -Pi :8001 -sTCP:LISTEN
        exit 1
    fi
    echo -e "${GREEN}✅ Port 8001 is now free${NC}"
fi

# Start HTTP server
echo -e "${GREEN}🚀 Starting web server on port 8001...${NC}"
echo ""
echo -e "${BLUE}╔════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║                     Access Instructions                    ║${NC}"
echo -e "${BLUE}╠════════════════════════════════════════════════════════════╣${NC}"
echo -e "${BLUE}║ 1. Forward port 8001 in VS Code:                          ║${NC}"
echo -e "${BLUE}║    - Open 'Ports' tab in VS Code                          ║${NC}"
echo -e "${BLUE}║    - Forward port 8001                                     ║${NC}"
echo -e "${BLUE}║    - Set visibility to 'Public' if needed                 ║${NC}"
echo -e "${BLUE}║                                                            ║${NC}"
echo -e "${BLUE}║ 2. Open the forwarded URL in your browser                 ║${NC}"
echo -e "${BLUE}║    (Chrome, Edge, or Opera required for Web Serial API)   ║${NC}"
echo -e "${BLUE}║                                                            ║${NC}"
echo -e "${BLUE}║ 3. Connect your ESP32 via USB to your computer            ║${NC}"
echo -e "${BLUE}║                                                            ║${NC}"
echo -e "${BLUE}║ 4. Click 'Connect and Flash ESP32'                        ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${GREEN}📡 Server running at: http://localhost:8001/${NC}"
echo -e "${GREEN}   (auto-redirects to web-flasher.html)${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop${NC}"
echo ""

cd "$PROJECT_DIR"

# Auto-restart loop for robustness
RESTART_COUNT=0
MAX_RESTARTS=5

while true; do
    # Start the server
    python3 "$SCRIPT_DIR/flasher_server.py"
    EXIT_CODE=$?
    
    # If exit code is 0 (Ctrl+C), exit cleanly
    if [ $EXIT_CODE -eq 0 ] || [ $EXIT_CODE -eq 130 ]; then
        echo -e "${GREEN}Server stopped cleanly${NC}"
        break
    fi
    
    # If server crashed, try to restart
    RESTART_COUNT=$((RESTART_COUNT + 1))
    
    if [ $RESTART_COUNT -ge $MAX_RESTARTS ]; then
        echo -e "${RED}❌ Server crashed $MAX_RESTARTS times, giving up${NC}"
        exit 1
    fi
    
    echo -e "${YELLOW}⚠️  Server crashed (exit code: $EXIT_CODE)${NC}"
    echo -e "${YELLOW}🔄 Auto-restarting... (attempt $RESTART_COUNT/$MAX_RESTARTS)${NC}"
    sleep 2
done
