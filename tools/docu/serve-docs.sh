echo "Serving documentation at http://localhost:8000 ..."
#!/bin/bash
# Serve Sphinx documentation locally
# Usage: ./serve-docs.sh
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_DIR/docs/_build/html"

# Check if build exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ Error: Documentation not built yet"
    echo "Run ./tools/docu/build-docs.sh first"
    exit 1
fi

echo "Serving documentation at http://localhost:8000 from $BUILD_DIR ..."
cd "$BUILD_DIR"

# Use nohup to keep server running after parent process exits
nohup python3 -m http.server 8000 > "$PROJECT_DIR/temp/docserver.log" 2>&1 &
echo "Documentation server running in background (log: temp/docserver.log)"
