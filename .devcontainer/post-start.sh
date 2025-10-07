#!/bin/bash
# Auto-start script for QEMU network infrastructure
# This runs automatically when Codespace starts
#
# Services started:
# 1. TUN Bridge (waits for QEMU to be available)
# 2. HTTP Proxy (retries until ESP32 is reachable)
#
# Both services run in background with quiet mode enabled.
# Errors are logged to /workspaces/esp32-distance/temp/*_errors.log

set -e

PROJECT_DIR="/workspaces/esp32-distance"
LOG_DIR="${PROJECT_DIR}/temp"

# Python environment used by ESP-IDF extension (may not exist on CI)
IDF_PYTHON="/opt/esp/python_env/idf5.4_py3.12_env/bin/python"

# Helper to run pip install if python exists
pip_install() {
	local pkg="$1"
	if [ -x "${IDF_PYTHON}" ]; then
		echo "Installing ${pkg} into ESP-IDF Python environment..."
		"${IDF_PYTHON}" -m pip install --no-warn-script-location --upgrade "${pkg}" || true
	else
		# Fall back to system python if available
		if command -v python3 >/dev/null 2>&1; then
			echo "Installing ${pkg} into system python3 environment..."
			python3 -m pip install --user --upgrade "${pkg}" || true
		else
			echo "No python found to install ${pkg}; skipping"
		fi
	fi
}

# Ensure log directory exists
mkdir -p "${LOG_DIR}"

echo "Ensuring QEMU network infrastructure is ready..."

# Use the idempotent ensure-network-stack script
# This script checks if services are already running before starting them
"${PROJECT_DIR}/tools/ensure-network-stack.sh"

echo ""
echo "✓ Network infrastructure ready"
echo ""
echo "Ensuring dev tools: pre-commit"
echo "(mkdocs already available in /opt/venv)"

# Create a local virtualenv for dev tools if it doesn't exist
DEV_VENV="${HOME}/.venv-devtools"
if [ ! -d "${DEV_VENV}" ]; then
  echo "Creating dev tools virtualenv..."
  python3 -m venv "${DEV_VENV}" || true
fi

# Install pre-commit into the dev venv (mkdocs already in /opt/venv)
if [ -d "${DEV_VENV}" ]; then
  echo "Installing pre-commit into dev virtualenv..."
  "${DEV_VENV}/bin/pip" install --quiet --upgrade pre-commit || true
fi

# Ensure pre-commit hooks are installed for the repo
# Use the direct path to pre-commit binary instead of relying on PATH
if [ -x "${DEV_VENV}/bin/pre-commit" ]; then
  echo "Installing git hooks via pre-commit"
  (cd "${PROJECT_DIR}" && "${DEV_VENV}/bin/pre-commit" install --install-hooks) || true
  echo "✓ Git hooks installed"
else
  echo "⚠ pre-commit not available; hooks not installed"
fi

echo "To access webserver:" 
echo "  1. Start QEMU: Click 'Debug' button in VS Code"
echo "  2. Open forwarded port 8080 in browser"
echo ""
echo "To check service status:"
echo "  ps aux | grep -E '(serial_tun_bridge|http_proxy)'"
echo ""
echo "To stop services:"
echo "  sudo pkill -f serial_tun_bridge"
echo "  pkill -f http_proxy"
echo ""

