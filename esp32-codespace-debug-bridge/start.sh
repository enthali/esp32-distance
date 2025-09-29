#!/bin/bash

# ESP32 Codespaces Debug Bridge - Start Script
# Starts both the bridge server and web client HTTP server

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BRIDGE_DIR="$SCRIPT_DIR/bridge-server"
WEB_CLIENT_DIR="$SCRIPT_DIR"

echo "🚀 Starting ESP32 Codespaces Debug Bridge..."
echo "📁 Script directory: $SCRIPT_DIR"

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "🛑 Stopping services..."
    pkill -f "node bridge.js" 2>/dev/null || true
    pkill -f "python3 -m http.server 9080" 2>/dev/null || true
    echo "✅ Cleanup complete"
    exit 0
}

# Trap Ctrl+C and cleanup
trap cleanup SIGINT SIGTERM

# Check if Node.js is available
if ! command -v node &> /dev/null; then
    echo "❌ Error: Node.js not found. Please install Node.js first."
    exit 1
fi

# Check if Python3 is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Error: Python3 not found. Please install Python3 first."
    exit 1
fi

# Check if bridge-server directory exists
if [ ! -d "$BRIDGE_DIR" ]; then
    echo "❌ Error: Bridge server directory not found: $BRIDGE_DIR"
    exit 1
fi

# Check if node_modules exists
if [ ! -d "$BRIDGE_DIR/node_modules" ]; then
    echo "📦 Installing npm dependencies..."
    cd "$BRIDGE_DIR"
    npm install
    if [ $? -ne 0 ]; then
        echo "❌ Error: Failed to install npm dependencies"
        exit 1
    fi
fi

echo ""
echo "🌉 Starting Bridge Server (WebSocket ↔ TCP)..."
cd "$BRIDGE_DIR"
node bridge.js &
BRIDGE_PID=$!

# Wait a moment for bridge to start
sleep 2

# Check if bridge started successfully
if ! kill -0 $BRIDGE_PID 2>/dev/null; then
    echo "❌ Error: Bridge server failed to start"
    exit 1
fi

echo "✅ Bridge Server started (PID: $BRIDGE_PID)"
echo "   • WebSocket Server: ws://localhost:9081"
echo "   • TCP Server: localhost:3333"

echo ""
echo "🌐 Starting Web Client HTTP Server..."
cd "$WEB_CLIENT_DIR/web-client"
python3 -m http.server 9080 &
HTTP_PID=$!

# Wait a moment for HTTP server to start
sleep 1

# Check if HTTP server started successfully
if ! kill -0 $HTTP_PID 2>/dev/null; then
    echo "❌ Error: HTTP server failed to start"
    cleanup
    exit 1
fi

echo "✅ Web Client HTTP Server started (PID: $HTTP_PID)"
echo "   • Web Interface: http://localhost:9080"

echo ""
echo "🎯 ESP32 Debug Bridge is ready!"
echo ""
echo "📋 Next Steps:"
echo "   1. Open http://localhost:9080 in your browser"
echo "   2. Connect to ESP32 serial port using Web Serial API"
echo "   3. Use GDB: xtensa-esp32-elf-gdb build/distance.elf -ex \"target remote localhost:3333\""
echo ""
echo "⚡ Active Services:"
echo "   • Web Client: http://localhost:9080"
echo "   • WebSocket Bridge: ws://localhost:9081"
echo "   • GDB Target: localhost:3333"
echo ""
echo "Press Ctrl+C to stop all services..."

# Wait for user to stop
wait