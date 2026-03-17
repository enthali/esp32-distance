Temperature Sensor User Story
==============================

.. story:: Temperature-Aware Parking Environment Indicator
   :id: US_TEMP_1
   :status: approved
   :links: US_DISPLAY_1, US_SETUP_1, REQ_SYS_TEMP_1, REQ_TEMP_1, REQ_TEMP_2, REQ_TEMP_3, REQ_TEMP_4, REQ_TEMP_5, REQ_SNS_15, REQ_DSP_6, REQ_WEB_TEMP_1, REQ_CFG_JSON_16
   :tags: temperature, sensor, garage-user, display

   **Role:** Garage user

   **Story:**
   As a garage user, I want the device to measure the garage temperature and
   show it on the web interface and through the LED colour, so that I can see
   at a glance whether it is cold (blue) or warm (orange) in the garage.

   **Value:**
   In a garage parking aid context, temperature affects both sensor accuracy and
   usability — icy conditions (< 5°C) are relevant for safety. The LED position
   indicator double-codes distance (position) and temperature (colour), providing
   ambient context without an extra display. The web interface gives precise
   temperature readings for remote monitoring.
