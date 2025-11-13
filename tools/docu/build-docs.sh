#!/bin/bash
# Build Sphinx documentation (HTML)
# Usage: ./build-docs.sh
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
DOCS_DIR="$PROJECT_DIR/docs"
BUILD_DIR="$DOCS_DIR/_build/html"
echo "Building Sphinx documentation in $BUILD_DIR ..."
sphinx-build -b html "$DOCS_DIR" "$BUILD_DIR"
