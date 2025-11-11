#!/bin/bash
# Serve Sphinx documentation with live auto-rebuild
# Usage: ./serve-docs.sh
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
DOCS_DIR="$PROJECT_DIR/docs"
BUILD_DIR="$PROJECT_DIR/docs/_build/html"

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

echo "Documentation server running in background (log: temp/docserver.log)"
echo "PID: $!"
