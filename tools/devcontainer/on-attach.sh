#!/bin/bash
# on-attach.sh

# This script runs every time you attach to the container

# Write timestamp to workspace root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_DIR"
echo "on-attach.sh executed at $(date)" > ./temp/on-attach-ran