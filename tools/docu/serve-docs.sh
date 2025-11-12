#!/bin/bash
# Serve Sphinx documentation with live auto-rebuild
# Usage: ./serve-docs.sh
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
DOCS_DIR="$PROJECT_DIR/docs"
BUILD_DIR="$PROJECT_DIR/docs/_build/html"
PID_FILE="$PROJECT_DIR/temp/docserver.pid"

# Kill any existing documentation server
if [ -f "$PID_FILE" ]; then
    OLD_PID=$(cat "$PID_FILE")
    if ps -p "$OLD_PID" > /dev/null 2>&1; then
        echo "Stopping existing documentation server (PID: $OLD_PID)..."
        kill "$OLD_PID" 2>/dev/null || true
        sleep 1
    fi
    rm -f "$PID_FILE"
fi

# Also kill any sphinx-autobuild processes on port 8000
pkill -f "sphinx-autobuild.*port 8000" 2>/dev/null || true
sleep 1

echo "Starting sphinx-autobuild with live preview and auto-rebuild..."
echo "Documentation will be available at http://localhost:8000"
echo "Changes to .rst files will trigger automatic rebuild"

# Use nohup to keep server running after parent process exits
# sphinx-autobuild watches for changes and rebuilds automatically
nohup /opt/venv/bin/sphinx-autobuild \
    "$DOCS_DIR" \
    "$BUILD_DIR" \
    --host 0.0.0.0 \
    --port 8000 \
    --no-initial \
    > "$PROJECT_DIR/temp/docserver.log" 2>&1 &

DOC_PID=$!
echo "$DOC_PID" > "$PID_FILE"
echo "Documentation server running in background (log: temp/docserver.log)"
echo "PID: $DOC_PID (saved to temp/docserver.pid)"
