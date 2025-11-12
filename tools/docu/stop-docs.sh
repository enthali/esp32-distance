#!/bin/bash
# Stop the documentation server
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
PID_FILE="$PROJECT_DIR/temp/docserver.pid"

echo "Stopping documentation server..."

# Stop via PID file
if [ -f "$PID_FILE" ]; then
    OLD_PID=$(cat "$PID_FILE")
    if ps -p "$OLD_PID" > /dev/null 2>&1; then
        echo "Stopping server (PID: $OLD_PID)..."
        kill "$OLD_PID" 2>/dev/null || true
        sleep 1
        
        # Force kill if still running
        if ps -p "$OLD_PID" > /dev/null 2>&1; then
            echo "Force killing server..."
            kill -9 "$OLD_PID" 2>/dev/null || true
        fi
    fi
    rm -f "$PID_FILE"
    echo "Documentation server stopped."
else
    echo "No PID file found."
fi

# Clean up any remaining sphinx-autobuild processes
pkill -f "sphinx-autobuild.*port 8000" 2>/dev/null && echo "Cleaned up remaining processes." || echo "No remaining processes found."
