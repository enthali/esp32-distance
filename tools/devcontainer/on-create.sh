#!/bin/bash
# on-create.sh 
# VNC Setup Script for ESP32 Template
# This script configures VNC for GUI access in the development container

echo "Setting up VNC configuration..."

# Create VNC directory
mkdir -p ~/.vnc

# Create optimized xstartup script with XFCE4
cat > ~/.vnc/xstartup << 'EOF'
#!/bin/bash
# VNC Startup Script with XFCE4 Desktop Environment
unset SESSION_MANAGER
unset DBUS_SESSION_BUS_ADDRESS

# Start dbus session
if which dbus-launch >/dev/null 2>&1; then
    eval `dbus-launch --sh-syntax --exit-with-session`
fi

# Set German keyboard layout for X11 session
setxkbmap de

# Start XFCE4 desktop environment
exec startxfce4
EOF

# Make startup script executable
chmod +x ~/.vnc/xstartup

echo "VNC configuration completed successfully!"
echo "Keyboard layout: German (DE) - can be changed with 'setxkbmap [layout]'"
echo "VNC should start automatically via desktop-lite feature"
echo "Access via noVNC: http://localhost:6080/vnc.html"

# Write timestamp to workspace root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_DIR"

# Create temp directory for logs and runtime files
mkdir -p "$PROJECT_DIR/temp"

echo "on-create.sh executed at $(date)" > ./temp/on-create-ran