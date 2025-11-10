#!/bin/bash
# on-start.sh

# This script ensures necessary permissions for user:esp operation in the development container
sudo chmod 1777 /tmp

# Determine project directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_DIR"

# Start documentation web server
echo ""
echo "Starting documentation web server..."
bash "$PROJECT_DIR/tools/docu/serve-docs.sh"
echo "Access documentation at http://localhost:8000"

# Write timestamp to workspace root
echo "on-start.sh executed at $(date)" > ./temp/on-start-ran
