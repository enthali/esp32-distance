Hardware Specifications
=======================

This page describes the hardware components, requirements, and pin configuration for the ESP32 Distance Sensor project.

Hardware Components
-------------------

Required Components
~~~~~~~~~~~~~~~~~~~

ESP32 Development Board
^^^^^^^^^^^^^^^^^^^^^^^

* **Model**: ESP32-WROOM-32 or compatible
* **Flash**: 4MB minimum (project optimized for 4MB modules)
* **RAM**: 520KB SRAM (standard ESP32)
* **WiFi**: 2.4 GHz 802.11 b/g/n
* **Power**: 5V via USB or external supply

**Recommended Boards:**

* ESP32 DevKitC
* ESP32-WROOM-32 Development Board
* NodeMCU-32S
* Any ESP32 board with accessible GPIO pins

WS2812 LED Strip
^^^^^^^^^^^^^^^^

* **Type**: Individually addressable RGB LEDs (NeoPixel compatible)
* **Quantity**: 40 LEDs (configurable in software)
* **Voltage**: 5V
* **Data Interface**: Single-wire serial protocol
* **Current**: ~60mA per LED at full brightness (2.4A total for 40 LEDs)

**Purchase Considerations:**

* Pre-soldered strips with connectors are easier to work with
* Waterproof (IP65) versions available for outdoor use
* Individual LEDs can be used instead of strips

HC-SR04 Ultrasonic Sensor
^^^^^^^^^^^^^^^^^^^^^^^^^^

* **Measurement Range**: 2cm - 400cm
* **Accuracy**: ±3mm
* **Operating Voltage**: 5V (works with 3.3V trigger)
* **Interface**: Digital trigger and echo pins
* **Update Rate**: Up to 50Hz (project uses 10Hz)

**Features:**

* Non-contact distance measurement
* 15° measurement angle
* Compact and affordable

Cables & Connectors
^^^^^^^^^^^^^^^^^^^

* **USB Cable**: Micro-USB or USB-C (depending on ESP32 board)
* **Jumper Wires**: Male-to-female for sensor connections
* **LED Strip Connectors**: JST connectors (often included with LED strips)

Optional Components
~~~~~~~~~~~~~~~~~~~

External 5V Power Supply
^^^^^^^^^^^^^^^^^^^^^^^^

**Recommended if:**

* Using all 40 LEDs at high brightness
* USB power insufficient (brownout issues)
* Deploying in permanent installation

**Specifications:**

* Voltage: 5V DC
* Current: 3A minimum (5A recommended for full brightness)
* Connector: Barrel jack or screw terminals

Power Distribution
^^^^^^^^^^^^^^^^^^

* Breadboard or perfboard for clean connections
* Voltage regulator (if using higher voltage supply)
* Capacitors for power filtering (100µF, 0.1µF)

Enclosure
^^^^^^^^^

* Waterproof box for outdoor installations
* Mounting hardware for garage ceiling/wall
* Cable management clips

Pin Configuration
-----------------

GPIO Pin Assignments
~~~~~~~~~~~~~~~~~~~~

.. list-table:: Pin Configuration
   :header-rows: 1
   :widths: 25 15 15 45

   * - Component
     - Function
     - GPIO Pin
     - Notes
   * - WS2812 LED Strip
     - Data
     - GPIO12
     - RMT peripheral for precise timing
   * - HC-SR04 Sensor
     - Trigger
     - GPIO14
     - Output: 10µs pulse to trigger measurement
   * - HC-SR04 Sensor
     - Echo
     - GPIO13
     - Input: High pulse duration = distance
   * - Power
     - VCC
     - 5V
     - From USB or external supply
   * - Ground
     - GND
     - GND
     - Common ground for all components

Wiring Diagram
~~~~~~~~~~~~~~

**ESP32 to HC-SR04 Sensor:**

.. code-block:: text

   ESP32           HC-SR04
   GPIO14   ---→   Trig
   GPIO13   ←---   Echo
   5V       ---→   VCC
   GND      ---→   GND

**ESP32 to WS2812 LED Strip:**

.. code-block:: text

   ESP32           WS2812 Strip
   GPIO12   ---→   DIN (Data In)
   5V       ---→   5V / VCC
   GND      ---→   GND

**Power Considerations:**

* **USB Power**: Sufficient for testing and low LED brightness
* **External 5V**: Connect to ESP32 VIN (not 3.3V pin) and LED strip VCC
* **Common Ground**: All components must share the same GND connection

.. warning::

   **Current Draw:** WS2812 LEDs can draw up to 60mA each at full white brightness. 
   40 LEDs = 2.4A total. USB ports typically provide 500-900mA. For full brightness, 
   use an external 5V power supply rated for at least 3A.

Pin Selection Rationale
~~~~~~~~~~~~~~~~~~~~~~~

**GPIO12 (LED Strip):**

* Supports RMT (Remote Control) peripheral for precise timing
* WS2812 requires precise 800kHz timing (T0H=0.4µs, T1H=0.8µs)
* RMT hardware generates timing automatically, no CPU overhead

**GPIO13/14 (HC-SR04):**

* General-purpose GPIO pins without special constraints
* Not used by flash, UART, or other critical functions
* Adjacent pins for clean wiring

**Avoided Pins:**

* GPIO0, GPIO2: Boot mode pins (pulled up/down at boot)
* GPIO1, GPIO3: UART0 (console/programming)
* GPIO6-11: Connected to flash memory (do not use)

Technical Specifications
------------------------

System Requirements
~~~~~~~~~~~~~~~~~~~

.. list-table:: Technical Specifications
   :header-rows: 1
   :widths: 30 70

   * - Specification
     - Value
   * - **Microcontroller**
     - ESP32-WROOM-32 (240MHz dual-core, 4MB flash, 520KB RAM)
   * - **LED Strip**
     - 40× WS2812 individually addressable RGB LEDs
   * - **Distance Sensor**
     - HC-SR04 ultrasonic (2cm-400cm range, ±3mm accuracy)
   * - **Update Rate**
     - 10Hz sensor readings, real-time LED updates
   * - **WiFi**
     - 2.4 GHz 802.11 b/g/n, AP and STA modes
   * - **Web Interface**
     - HTTP server (HTTPS not yet implemented)
   * - **Power Consumption**
     - ~80mA (ESP32) + up to 2.4A (LEDs at full brightness)
   * - **Operating Voltage**
     - 5V (3.3V logic levels for ESP32)
   * - **Memory Usage**
     - ~59% flash, ~18% DRAM, ~76% IRAM (optimized for 4MB)
   * - **Development Framework**
     - ESP-IDF v5.4.1, FreeRTOS

Performance Characteristics
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Distance Measurement:**

* Range: 10cm - 50cm (configurable)
* Resolution: 1mm (millimeter precision)
* Update Rate: 10Hz (100ms per reading)
* Accuracy: ±3mm (sensor specification)

**LED Display:**

* LEDs: 40 individually addressable
* Colors: 16.7 million (24-bit RGB)
* Brightness: 0-255 per channel (configurable)
* Response Time: <10ms (real-time feedback)

**Network:**

* WiFi Modes: AP (Access Point) and STA (Station)
* AP SSID: "ESP32-Distance-Sensor" (default)
* Web Server: HTTP on port 80
* Configuration: Captive portal for easy setup

Bill of Materials (BOM)
------------------------

.. list-table:: Bill of Materials
   :header-rows: 1
   :widths: 10 30 15 15 30

   * - Qty
     - Component
     - Ref
     - Est. Cost
     - Notes
   * - 1
     - ESP32 Development Board
     - U1
     - $8-15
     - ESP32-WROOM-32 or compatible
   * - 1
     - WS2812 LED Strip (40 LEDs)
     - LED1
     - $10-20
     - 5V, individually addressable
   * - 1
     - HC-SR04 Ultrasonic Sensor
     - U2
     - $2-5
     - Standard HC-SR04 module
   * - 1
     - USB Cable (Micro-USB/USB-C)
     - —
     - $3-8
     - For programming and power
   * - 1
     - Jumper Wires (M-F, 10cm)
     - —
     - $2-5
     - Assorted pack
   * - 1
     - 5V Power Supply (3A+)
     - PS1
     - $8-15
     - Optional, for full LED brightness
   * - 
     - **Total Estimated Cost**
     - 
     - **$33-68**
     - Prices vary by region and supplier

.. note::

   **Budget Option:** Start with just ESP32 + sensor + a few LEDs for initial testing. 
   Add the full LED strip later when you're ready for the complete display.

Hardware Assembly
-----------------

Basic Assembly Steps
~~~~~~~~~~~~~~~~~~~~

1. **Prepare Components**

   * Identify all GPIO pins on ESP32 board
   * Check LED strip polarity (DIN, 5V, GND)
   * Verify HC-SR04 sensor pin labels (Trig, Echo, VCC, GND)

2. **Connect HC-SR04 Sensor**

   * Trig → GPIO14
   * Echo → GPIO13
   * VCC → 5V
   * GND → GND

3. **Connect WS2812 LED Strip**

   * DIN (Data In) → GPIO12
   * 5V/VCC → 5V (or external power supply)
   * GND → GND (common ground with ESP32)

4. **Power Considerations**

   * **USB Power Only:** Connect ESP32 via USB, LED strip to ESP32 5V pin
   * **External Power:** Connect 5V supply to both ESP32 VIN and LED strip VCC, share GND

5. **Verify Connections**

   * Double-check all pin assignments
   * Ensure common ground between all components
   * Check for loose connections

Safety Considerations
~~~~~~~~~~~~~~~~~~~~~

.. warning::

   **Electrical Safety:**

   * Do not exceed 5V on ESP32 VIN or LED strip
   * Use proper gauge wires for high-current LED connections
   * External power supply should have short-circuit protection
   * Never connect/disconnect components while powered on

.. note::

   **Best Practices:**

   * Use a breadboard for prototyping before permanent installation
   * Add a 100µF capacitor across LED strip power pins (reduces noise)
   * Keep sensor away from ultrasonic-interfering materials (foam, fabric)
   * Mount LED strip away from direct sunlight (improves visibility)

Next Steps
----------

Hardware ready? Great! Now you can:

1. **Build the Firmware** - See :doc:`quick-start` for build instructions
2. **Flash to ESP32** - Upload the firmware via USB
3. **Test the System** - Verify sensor readings and LED display
4. **Configure WiFi** - Access web interface for configuration

**No hardware yet?** No problem! Try :doc:`../90_guides/qemu-emulator` for full simulation.
