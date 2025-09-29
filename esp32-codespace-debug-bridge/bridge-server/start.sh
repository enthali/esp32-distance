#!/bin/bash

# ESP32 Codespace Debug Bridge Startup Script

echo "🚀 Starting ESP32 Codespace Debug Bridge..."

# Check if Node.js is available
if ! command -v node &> /dev/null; then
    echo "❌ Node.js not found. Please install Node.js first."
    exit 1
fi

# Install dependencies if node_modules doesn't exist
if [ ! -d "node_modules" ]; then
    echo "📦 Installing dependencies..."
    npm install
fi

# Start the bridge server
echo "🔌 Starting bridge server..."
echo "Press Ctrl+C to stop"
echo ""

node bridge.js