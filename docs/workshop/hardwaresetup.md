# Hardware Setup

This guide explains the hardware required for the ESP32 Distance Sensor Workshop and how to wire up the components.

![Setup](../assets/images/setup.jpg)

## Required hardware

### ESP32 WROOM-32F development board (4MB flash) in this picture its a ESP32_Relay X2 board

![ESP32](../assets/images/ESP32_Relais_X2.jpg)

### HC-SR04 ultrasonic distance sensor

![HC-SR04](../assets/images/UltrasonicSensor.jpg)

### WS2812 addressable LED strip (40 LEDs recommended)

![LED Strip](../assets/images/WS2812Stripe.jpg)

### USB serial cable for programming here a CH340 USB-A to TTL-Serial adapter

![USB Serial Adapter](../assets/images/USB%20Serial.jpg)

## Wiring overview
- Connect the HC-SR04 trigger pin to the **GPIO13 PIN** and the echo pin to **GPIO12 pin** on the ESP32. 
- Connect the HC-SR04 power to **the 5V pin** and ground to **any GND pin** of the ESP32 board. For details, see [requirements](../requirements/distance-sensor-requirements.md).
- Connect the WS2812 data line to the ESP32 **GPIO12 pin** (see also [requirements](../requirements/led-controller-requirements.md)).
- Connect the WS2812 ground to a **GND pin**
- Connect the WS2812 **5V Vcc** to ... well ... here's the thing the Relay X2 has no additional 5V pin available, but wait, there is a **3,3 V pin** and it works just fine.
- Connect the ESP32 module to GND and 5V of the USP Serial Adapter and **crossconnect** the two **TX x RX** lines
```
    +-------------------+         +-------------------------+
    |   USB Serial      |         |      ESP32 Board        |
    |   Adapter (CH340) |         |    (ESP32 WROOM-32F)    |
    +-------------------+         +-------------------------+
    |                   |         |                         |
    |   5V   -----------+---------+--- 5V                   |
    |   GND  -----------+---------+--- GND                  |
    |                   |         |                         |
    |   TX   -----------+--\   /--+--- TX (GPIO3/U0RXD)     |
    |                   |   \ /   |                         |
    |                   |   / \   |                         |
    |   RX   -----------+--/   \--+--- RX (GPIO1/U0TXD)     |
    |                   |         |                         |
    +-------------------+         +-------------------------+
```
**Note:** TX on the adapter connects to RX on the ESP32, and RX on the adapter connects to TX on the ESP32 (crossed lines).

> **Tip:** Double-check your wiring before plugging the USB Serial Adapter into your PC or Laptop......
