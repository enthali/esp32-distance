#!/bin/bash
# Auto-start VNC Server
# This script starts the VNC server automatically if not already running

VNC_DISPLAY=":1"
VNC_GEOMETRY="1024x768"
VNC_DEPTH="24"

# Check if VNC server is already running
if vncserver -list | grep -q "^1"; then
    echo "VNC server already running on display $VNC_DISPLAY"
    exit 0
fi

# Start VNC server
echo "Starting VNC server on display $VNC_DISPLAY..."
vncserver $VNC_DISPLAY -geometry $VNC_GEOMETRY -depth $VNC_DEPTH

# Set DISPLAY for current session
export DISPLAY=$VNC_DISPLAY
echo "export DISPLAY=$VNC_DISPLAY" >> ~/.bashrc

echo "VNC server started successfully!"
echo "Access GUI via NoVNC on port 6080"
echo "Local DISPLAY set to: $DISPLAY"