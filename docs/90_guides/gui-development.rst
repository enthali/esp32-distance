GUI and Graphics Development
=============================

This guide explains how to develop graphical applications using the VNC/noVNC setup in the ESP32 Template development container.

Overview
--------

The dev container includes a complete desktop environment for GUI development:

- **Fluxbox Window Manager**: Lightweight desktop environment
- **TigerVNC Server**: VNC server running on display ``:1``
- **noVNC Web Client**: Browser-based VNC access
- **X11 Support**: Full X Window System for GUI applications

This setup allows you to:

- Run GUI applications from the terminal (they appear on the desktop)
- Access the desktop via web browser at port 6080
- Use VNC clients for better performance
- Develop and test graphical tools without leaving Codespaces

Architecture
------------

Desktop Stack
~~~~~~~~~~~~~

.. code-block:: text

   ┌─────────────────────────────────────────────┐
   │  Browser (noVNC) or VNC Client              │
   └──────────────────┬──────────────────────────┘
                      │ Port 6080 (HTTP+WebSocket)
                      │ Port 5901 (VNC Protocol)
   ┌──────────────────▼──────────────────────────┐
   │  noVNC Server (websockify)                  │
   │  - HTTP server for web interface            │
   │  - WebSocket → VNC protocol bridge          │
   └──────────────────┬──────────────────────────┘
                      │ VNC Protocol
   ┌──────────────────▼──────────────────────────┐
   │  TigerVNC Server (:1)                       │
   │  - DISPLAY=:1                               │
   │  - Resolution: 1024x768 @ 24-bit            │
   │  - TMPDIR=/home/esp/vnc-tmp                 │
   └──────────────────┬──────────────────────────┘
                      │ X11 Protocol
   ┌──────────────────▼──────────────────────────┐
   │  Fluxbox Window Manager                     │
   │  - Lightweight, minimal resource usage      │
   │  - Right-click for application menu         │
   └──────────────────┬──────────────────────────┘
                      │
   ┌──────────────────▼──────────────────────────┐
   │  X11 Applications                           │
   │  - GUI tools (gtkwave, etc.)                │
   │  - VS Code terminal commands                │
   │  - Custom graphical applications            │
   └─────────────────────────────────────────────┘

Accessing the Desktop
----------------------

Via Web Browser (Recommended for Codespaces)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. **Automatic Port Forwarding**: Codespaces automatically forwards port 6080
2. **Open in Browser**: Click on "Ports" tab → Port 6080 → Click globe icon
3. **Full Screen**: Use browser's full screen mode (F11) for better experience

.. note::
   noVNC is perfect for Codespaces as it requires no client installation and works in any browser.

Via VNC Client (Better Performance)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For local Docker containers or better performance:

.. code-block:: bash

   # Find your container's IP or use port forwarding
   # Connect VNC client to: localhost:5901
   # Password: (typically 'vscode' or none)

**Recommended VNC Clients:**

- **RealVNC Viewer** (cross-platform)
- **TigerVNC Viewer** (Linux)
- **TightVNC** (Windows)

VNC Setup Details
-----------------

Automatic Startup
~~~~~~~~~~~~~~~~~

VNC server starts automatically when the dev container launches:

- **Startup Script**: ``.devcontainer/setup-vnc.sh``
- **systemd Integration**: VNC runs as a service
- **TMPDIR Workaround**: Uses ``/home/esp/vnc-tmp`` instead of ``/tmp``

.. code-block:: bash

   # Check VNC status
   ps aux | grep vnc
   
   # View VNC log
   tail -f /home/esp/.vnc/*.log
   
   # Restart VNC if needed
   vncserver -kill :1
   vncserver :1 -geometry 1024x768 -depth 24

Key Configuration Files
~~~~~~~~~~~~~~~~~~~~~~~

**``.devcontainer/setup-vnc.sh``**:

.. code-block:: bash

   #!/bin/bash
   # VNC server startup script
   
   export TMPDIR=/home/esp/vnc-tmp
   mkdir -p $TMPDIR
   
   vncserver :1 \
     -geometry 1024x768 \
     -depth 24 \
     -localhost no \
     -SecurityTypes None

**``.vnc/xstartup``**:

.. code-block:: bash

   #!/bin/bash
   unset SESSION_MANAGER
   unset DBUS_SESSION_BUS_ADDRESS
   
   export TMPDIR=/home/esp/vnc-tmp
   
   [ -x /etc/vnc/xstartup ] && exec /etc/vnc/xstartup
   [ -r $HOME/.Xresources ] && xrdb $HOME/.Xresources
   
   xsetroot -solid grey
   exec fluxbox

TMPDIR Workaround
~~~~~~~~~~~~~~~~~

**Problem**: Codespaces mounts ``/tmp`` as read-only, causing VNC to fail.

**Solution**: Use custom TMPDIR for VNC temporary files.

.. code-block:: bash

   # Environment variable set in all VNC-related scripts
   export TMPDIR=/home/esp/vnc-tmp
   
   # Directory is writable by container user
   mkdir -p /home/esp/vnc-tmp
   chmod 700 /home/esp/vnc-tmp

Running GUI Applications
------------------------

From VS Code Terminal
~~~~~~~~~~~~~~~~~~~~~

Any GUI application launched from the terminal appears on the VNC desktop:

.. code-block:: bash

   # Set display (usually automatic)
   export DISPLAY=:1
   
   # Launch GUI application
   firefox &
   
   # Launch VS Code (if available)
   code . &
   
   # Run custom GUI tool
   ./my_gui_app &

**Common GUI Tools in Container:**

- ``xterm``: Terminal emulator
- ``xcalc``: Calculator (for testing)
- File manager (if installed)
- Custom debugging tools

Example: GTKWave for Waveform Viewing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Install GTKWave (if needed)
   sudo apt-get update && sudo apt-get install -y gtkwave
   
   # Launch GTKWave
   gtkwave waveform.vcd &
   
   # Application window appears on VNC desktop

Developing Custom GUI Applications
-----------------------------------

ESP32 GUI Frameworks
~~~~~~~~~~~~~~~~~~~~

While the VNC desktop is for **host-side tools**, ESP32 can drive real displays:

**For ESP32 Display Output:**

- **LVGL** (Light and Versatile Graphics Library)
- **ESP-IDF LCD/Display Drivers**
- **SPI/I2C Display Libraries**

**Host Development Tools:**

- Use VNC desktop for testing GUI tool scripts
- Visualize sensor data with Python matplotlib
- Debug display layouts before deploying to ESP32

Python GUI Example
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   # gui_test.py - Simple GUI application
   import tkinter as tk
   
   root = tk.Tk()
   root.title("ESP32 Configuration Tool")
   
   label = tk.Label(root, text="Hello from VNC Desktop!")
   label.pack(pady=20)
   
   button = tk.Button(root, text="Close", command=root.quit)
   button.pack()
   
   root.mainloop()

.. code-block:: bash

   # Run the GUI app (appears on VNC desktop)
   python3 gui_test.py

C++ GUI with GTK (Advanced)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   // example_gtk.cpp
   #include <gtk/gtk.h>
   
   static void on_button_clicked(GtkWidget *widget, gpointer data) {
       g_print("Button clicked!\n");
   }
   
   int main(int argc, char *argv[]) {
       gtk_init(&argc, &argv);
       
       GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
       gtk_window_set_title(GTK_WINDOW(window), "ESP32 Tool");
       
       GtkWidget *button = gtk_button_new_with_label("Click Me");
       g_signal_connect(button, "clicked", 
                       G_CALLBACK(on_button_clicked), NULL);
       
       gtk_container_add(GTK_CONTAINER(window), button);
       gtk_widget_show_all(window);
       
       gtk_main();
       return 0;
   }

.. code-block:: bash

   # Compile and run
   g++ example_gtk.cpp -o example_gtk `pkg-config --cflags --libs gtk+-3.0`
   ./example_gtk &

Troubleshooting
---------------

VNC Server Not Starting
~~~~~~~~~~~~~~~~~~~~~~~

**Symptom**: Cannot connect to VNC on port 6080

.. code-block:: bash

   # Check if VNC server is running
   ps aux | grep vnc
   
   # Check VNC log for errors
   cat /home/esp/.vnc/*.log
   
   # Common issue: TMPDIR not writable
   ls -ld /home/esp/vnc-tmp
   
   # Restart VNC with correct TMPDIR
   export TMPDIR=/home/esp/vnc-tmp
   vncserver -kill :1
   vncserver :1 -geometry 1024x768 -depth 24

Cannot See GUI Applications
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Symptom**: Applications launch but don't appear on desktop

.. code-block:: bash

   # Verify DISPLAY is set correctly
   echo $DISPLAY  # Should show :1
   
   # Set DISPLAY if needed
   export DISPLAY=:1
   
   # Test with simple GUI
   xterm &
   
   # Check X server errors
   xdpyinfo -display :1

Black Screen in noVNC
~~~~~~~~~~~~~~~~~~~~~

**Symptom**: VNC connects but shows black screen

.. code-block:: bash

   # Fluxbox might not be running
   ps aux | grep fluxbox
   
   # Restart Fluxbox
   DISPLAY=:1 fluxbox &
   
   # Or restart entire VNC session
   vncserver -kill :1
   export TMPDIR=/home/esp/vnc-tmp
   vncserver :1 -geometry 1024x768 -depth 24

Performance Issues
~~~~~~~~~~~~~~~~~~

**Symptom**: Slow or laggy VNC connection

1. **Use native VNC client** instead of noVNC (better performance)
2. **Reduce color depth**: Change ``-depth 24`` to ``-depth 16``
3. **Lower resolution**: Use ``1024x768`` instead of higher resolutions
4. **Close unused applications** on the VNC desktop

Port 6080 Already in Use
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Find what's using the port
   sudo lsof -i :6080
   
   # Kill the process
   sudo kill -9 <PID>
   
   # Restart noVNC
   websockify --web=/usr/share/novnc 6080 localhost:5901 &

Best Practices
--------------

Development Workflow
~~~~~~~~~~~~~~~~~~~~

1. **Use Terminal First**: Most ESP-IDF work is command-line based
2. **GUI for Visualization**: Use VNC desktop for:
   
   - Waveform viewing (GTKWave)
   - Data plotting (Python matplotlib)
   - Custom debugging tools
   - Web browser testing

3. **Keep Desktop Clean**: Close unused GUI apps to save resources
4. **Persistent Sessions**: VNC survives terminal disconnections

Resource Management
~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Monitor container resource usage
   top
   
   # Check memory usage
   free -h
   
   # Kill heavy GUI processes if needed
   pkill firefox
   pkill chrome

Security Considerations
~~~~~~~~~~~~~~~~~~~~~~~

**VNC Security in Codespaces:**

- VNC runs without password (container is private)
- Codespaces port forwarding is authenticated via GitHub
- **Do NOT expose port 5901 publicly** without authentication

**For Production Use:**

.. code-block:: bash

   # Set VNC password
   vncpasswd
   
   # Start with security
   vncserver :1 -SecurityTypes VncAuth

Use Cases
---------

Common Development Scenarios
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**1. Testing Web Interfaces**

.. code-block:: bash

   # Build and run ESP32 web server in QEMU
   idf.py build
   ./tools/run-qemu-network.sh
   
   # Open browser on VNC desktop
   firefox http://10.0.2.15 &

**2. Visualizing Sensor Data**

.. code-block:: python

   # plot_sensor.py
   import matplotlib.pyplot as plt
   import serial
   
   # Read from ESP32 serial port
   data = []
   with serial.Serial('/dev/ttyUSB0', 115200) as ser:
       for _ in range(100):
           data.append(float(ser.readline()))
   
   # Plot on VNC desktop
   plt.plot(data)
   plt.show()

**3. Debugging with gtkwave**

.. code-block:: bash

   # Generate VCD file from logic analyzer
   idf.py build
   
   # View waveforms
   gtkwave debug/signals.vcd &

Integration with VS Code
------------------------

Opening Files from Desktop
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # From VNC terminal (xterm)
   code /workspaces/esp32-template/main/main.c
   
   # File opens in VS Code editor

Running VS Code Tasks
~~~~~~~~~~~~~~~~~~~~~

GUI applications can be launched from VS Code tasks:

.. code-block:: json

   // .vscode/tasks.json
   {
     "version": "2.0.0",
     "tasks": [
       {
         "label": "View Waveform",
         "type": "shell",
         "command": "DISPLAY=:1 gtkwave ${file} &",
         "problemMatcher": []
       }
     ]
   }

Advanced Configuration
----------------------

Custom Desktop Resolution
~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Stop current VNC server
   vncserver -kill :1
   
   # Start with custom resolution
   export TMPDIR=/home/esp/vnc-tmp
   vncserver :1 -geometry 1920x1080 -depth 24

Multiple VNC Displays
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Start second display
   vncserver :2 -geometry 800x600 -depth 24
   
   # Applications on different displays
   DISPLAY=:1 firefox &
   DISPLAY=:2 gtkwave waveform.vcd &

Alternative Window Managers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Replace Fluxbox with other window managers:

.. code-block:: bash

   # Install alternative WM
   sudo apt-get install -y openbox
   
   # Edit ~/.vnc/xstartup
   # Replace 'exec fluxbox' with 'exec openbox'

References
----------

- `TigerVNC Documentation <https://tigervnc.org/>`_
- `noVNC Project <https://novnc.com/>`_
- `Fluxbox Window Manager <http://fluxbox.org/>`_
- `X Window System <https://www.x.org/>`_

See Also
--------

- :doc:`devcontainer` - Dev container setup and configuration
- :doc:`debugging` - Debugging ESP32 applications
- :doc:`qemu-emulator` - QEMU emulation for ESP32
