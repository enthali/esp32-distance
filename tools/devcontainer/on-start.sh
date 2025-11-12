#!/bin/bash
# on-start.sh

# This script ensures necessary permissions for user:esp operation in the development container
sudo chmod 1777 /tmp

# Determine project directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_DIR"

# Ensure temp directory exists
mkdir -p "$PROJECT_DIR/temp"

# Start documentation web server (with cleanup of old instances)
echo ""
echo "Starting documentation web server..."
bash "$PROJECT_DIR/tools/docu/serve-docs.sh"

# Give the server a moment to start
sleep 2

# Verify server is running
if curl -s http://localhost:8000 > /dev/null 2>&1; then
    echo "✓ Documentation server is running at http://localhost:8000"
else
    echo "⚠ Documentation server may not be ready yet (check temp/docserver.log)"
fi

# Write timestamp to workspace root
echo "on-start.sh executed at $(date)" > ./temp/on-start-ran
