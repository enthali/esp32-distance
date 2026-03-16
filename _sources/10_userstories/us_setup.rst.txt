Setup User Story
=================

.. story:: Device Setup via Web Interface
   :id: US_SETUP_1
   :status: approved
   :links: REQ_SYS_NET_1, REQ_SYS_WEB_1, REQ_SYS_CFG_1, REQ_WEB_1, REQ_WEB_2, REQ_WEB_3, REQ_WEB_4, REQ_WEB_5, REQ_WEB_CONF_1, REQ_WEB_SCHEMA_1, REQ_WEB_LED_1, REQ_WEB_NF_1, REQ_WEB_NF_2, REQ_CFG_JSON_1, REQ_CFG_JSON_2, REQ_CFG_JSON_3, REQ_CFG_JSON_4, REQ_CFG_JSON_5, REQ_CFG_JSON_6
   :tags: setup, wifi, web, maker

   **Role:** Maker / builder

   **Story:**
   As a maker, I want to connect the device to my home WiFi and configure the key
   operating parameters through a web interface, so that I can get it up and running
   without touching any code.

   **Value:**
   The device creates its own access point on first boot. The maker connects, opens a
   browser, enters WiFi credentials and adjusts parameters like target distance and LED
   count. From that point the device is configured and joins the home network automatically.
