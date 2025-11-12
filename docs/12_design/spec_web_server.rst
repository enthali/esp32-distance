Web Server Design Specification
================================

This document specifies the design of the ESP32 web server component that provides HTTP-based user interface, WiFi configuration, and device configuration capabilities.


Architecture Overview
---------------------

.. spec:: Web Server Architecture
   :id: SPEC_WEB_ARCH_1
   :links: REQ_WEB_1, REQ_WEB_2, REQ_WEB_3, REQ_WEB_CONF_1
   :status: approved
   :tags: web, architecture

   **Overview:**
   The web server component provides a layered HTTP server architecture based on ESP-IDF's ``esp_http_server`` library. It serves three main pages (dashboard, WiFi setup, settings) plus REST API endpoints.

   **Architecture Layers:**

   1. **ESP-IDF HTTP Server Layer**:
      - ``httpd_start()`` with custom config (port, max connections, LRU purge)
      - Event-driven non-blocking connection handling
      - URI handler registration system

   2. **Static Content Layer**:
      - HTML/CSS/JS files embedded in flash using ``EMBED_FILES``
      - Binary symbols generated at build time (``_binary_xxx_start/end``)
      - MIME type detection based on file extension
      - Cache control headers (no-cache for development)

   3. **REST API Layer**:
      - WiFi management endpoints (``/scan``, ``/connect``, ``/status``, ``/reset``)
      - Configuration endpoints (``/api/config``, ``/api/config/reset``)
      - System health endpoint (``/api/system/health``)
      - JSON request/response using cJSON library

   4. **Integration Layer**:
      - Calls to ``wifi_manager`` component for WiFi operations
      - Calls to ``config_manager`` component for configuration access
      - Error code mapping from ESP-IDF to HTTP status codes

   **Component Dependencies:**
   
   - ``wifi_manager``: WiFi connection state, credentials, scanning
   - ``config_manager``: Configuration read/write, factory reset
   - ``esp_http_server``: HTTP protocol handling
   - ``cJSON``: JSON serialization/deserialization
   - ``cert_handler``: HTTPS support (future)

   **Data Flow Example (Schema-Driven Configuration Update):**

   ::

      Browser GET /api/config/schema
        ↓
      schema_get_handler() → config_get_schema_json()
        ↓
      Return embedded config_schema.json
        ↓
      Browser generates dynamic form from schema
        ↓
      Browser POST /api/config (flat JSON)
        ↓
      config_set_handler() → config_set_all_from_json()
        ↓
      Config manager parses JSON using schema for type validation
        ↓
      Bulk NVS write (all fields, single commit)
        ↓
      Return HTTP 200 with success response


Static File Embedding
----------------------

.. spec:: Static File Embedding Strategy
   :id: SPEC_WEB_STATIC_1
   :links: REQ_WEB_1, REQ_WEB_2, REQ_WEB_3, REQ_WEB_NF_1
   :status: approved
   :tags: web, build, embedding

   **Problem:**
   ESP32 has no filesystem by default. HTML/CSS/JS files must be embedded in firmware flash.

   **Solution:**
   Use ESP-IDF's ``EMBED_FILES`` CMake directive to convert static files into binary symbols at build time.

   **CMakeLists.txt Configuration:**

   .. code-block:: cmake

      idf_component_register(
          SRCS "web_server.c" "wifi_manager.c"
          INCLUDE_DIRS "."
          EMBED_FILES 
              "www/index.html"
              "www/wifi-setup.html" 
              "www/settings.html"
              "www/favicon.svg"
              "www/css/style.css"
              "www/js/app.js"
          REQUIRES 
              config_manager
              esp_wifi 
              esp_http_server 
              json
      )

   **Generated Binary Symbols:**

   For each file ``path/to/file.ext``, CMake generates:

   - ``_binary_file_ext_start``: Pointer to start of file data
   - ``_binary_file_ext_end``: Pointer to end of file data

   **C Code Access:**

   .. code-block:: c

      extern const uint8_t index_html_start[] asm("_binary_index_html_start");
      extern const uint8_t index_html_end[] asm("_binary_index_html_end");

      size_t size = index_html_end - index_html_start;
      httpd_resp_send(req, (const char*)index_html_start, size);

   **File Lookup Function:**

   The ``get_embedded_file()`` function maps URI paths to embedded file symbols:

   .. code-block:: c

      static esp_err_t get_embedded_file(const char *filename, 
                                         const uint8_t **data, 
                                         size_t *size)
      {
          // Strip query parameters (?v=2 for cache busting)
          char clean_filename[128];
          strncpy(clean_filename, filename, sizeof(clean_filename) - 1);
          char *query = strchr(clean_filename, '?');
          if (query != NULL) *query = '\0';

          if (strcmp(clean_filename, "/index.html") == 0) {
              *data = index_html_start;
              *size = index_html_end - index_html_start;
          }
          // ... more file mappings ...
          else {
              return ESP_ERR_NOT_FOUND;
          }
          return ESP_OK;
      }

   **MIME Type Detection:**

   .. code-block:: c

      static const char *get_mime_type(const char *filename)
      {
          const char *ext = strrchr(filename, '.');
          if (strcmp(ext, ".html") == 0) return "text/html";
          if (strcmp(ext, ".css") == 0) return "text/css";
          if (strcmp(ext, ".js") == 0) return "application/javascript";
          if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
          if (strcmp(ext, ".json") == 0) return "application/json";
          return "text/plain";
      }

   **Cache Control:**

   For template/development, caching is disabled:

   .. code-block:: c

      httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
      httpd_resp_set_hdr(req, "Pragma", "no-cache");
      httpd_resp_set_hdr(req, "Expires", "0");


URI Routing and Handlers
-------------------------

.. spec:: URI Routing Table
   :id: SPEC_WEB_ROUTES_1
   :links: REQ_WEB_4, REQ_WEB_5
   :status: approved
   :tags: web, routing

   **Handler Registration:**

   All URI handlers are registered in ``web_server_init()`` using ``httpd_register_uri_handler()``.

   **Main Pages:**

   =========================  ============  ================================  ===============
   URI                        HTTP Method   Handler Function                  Purpose
   =========================  ============  ================================  ===============
   ``/``                      GET           ``root_handler()``                Dashboard (redirects to /index.html)
   ``/index.html``            GET           ``static_file_handler()``         Main dashboard page
   ``/wifi-setup.html``       GET           ``static_file_handler()``         WiFi configuration page
   ``/settings.html``         GET           ``static_file_handler()``         Device settings page
   ``/config``                GET           ``config_handler()``              **Captive portal redirect** (→ /wifi-setup.html)
   =========================  ============  ================================  ===============

   **Static Assets:**

   =========================  ============  ================================  ===============
   URI                        HTTP Method   Handler Function                  Content Type
   =========================  ============  ================================  ===============
   ``/css/style.css``         GET           ``static_file_handler()``         text/css
   ``/js/app.js``             GET           ``static_file_handler()``         application/javascript
   ``/favicon.svg``           GET           ``static_file_handler()``         image/svg+xml
   ``/favicon.ico``           GET           ``static_file_handler()``         image/svg+xml (alias)
   =========================  ============  ================================  ===============

   **WiFi Management API:**

   =========================  ============  ================================  ===============
   URI                        HTTP Method   Handler Function                  Purpose
   =========================  ============  ================================  ===============
   ``/scan``                  GET           ``scan_handler()``                WiFi network scan
   ``/connect``               POST          ``connect_handler()``             Connect to WiFi network
   ``/reset``                 POST          ``reset_handler()``               Clear WiFi credentials, restart
   ``/api/status``            GET           ``wifi_status_handler()``         **WiFi status** (SSID, RSSI, mode)
   =========================  ============  ================================  ===============

   **Configuration Management API:**

   =========================  ============  ================================  ===============
   URI                        HTTP Method   Handler Function                  Purpose
   =========================  ============  ================================  ===============
   ``/api/config/schema``     GET           ``schema_get_handler()``          Get JSON schema for UI generation
   ``/api/config``            GET           ``config_get_handler()``          Get all configuration values (bulk)
   ``/api/config``            POST          ``config_set_handler()``          Update configuration values (bulk)
   ``/api/config/reset``      POST          ``config_reset_handler()``        Factory reset configuration
   =========================  ============  ================================  ===============

   **System Diagnostics API:**

   =========================  ============  ================================  ===============
   URI                        HTTP Method   Handler Function                  Purpose
   =========================  ============  ================================  ===============
   ``/api/system/health``     GET           ``system_health_handler()``       **System diagnostics** (uptime, memory, health)
   =========================  ============  ================================  ===============

   **CORS Support:**

   =========================  ============  ================================  ===============
   URI                        HTTP Method   Handler Function                  Purpose
   =========================  ============  ================================  ===============
   ``/api/*``                 OPTIONS       ``cors_preflight_handler()``      CORS preflight
   =========================  ============  ================================  ===============

   **Handler Registration Pattern:**

   .. code-block:: c

      httpd_uri_t scan_uri = {
          .uri = "/scan",
          .method = HTTP_GET,
          .handler = scan_handler,
          .user_ctx = NULL
      };
      esp_err_t ret = httpd_register_uri_handler(server, &scan_uri);
      ESP_LOGI(TAG, "Registered handler for '/scan' - %s", 
               ret == ESP_OK ? "OK" : esp_err_to_name(ret));

   **Configuration:**

   - ``max_uri_handlers``: 32 (increased from default 8)
   - ``lru_purge_enable``: true (automatically remove least recently used handlers if limit reached)


Configuration API Design
--------------------------

.. spec:: Configuration API Endpoints
   :id: SPEC_WEB_REST_CFG_1
   :links: REQ_WEB_CONF_1, REQ_WEB_SCHEMA_1
   :status: approved
   :tags: web, api, config, schema

   **Architecture:**
   The web server provides HTTP transport for configuration operations. All configuration logic is delegated to the config manager component.

   **Endpoints:**

   1. ``GET /api/config/schema`` - Retrieve JSON schema for UI generation
   2. ``GET /api/config`` - Retrieve all configuration values  
   3. ``POST /api/config`` - Update configuration values

   ---

   **Endpoint: GET /api/config/schema**

   **Purpose:** Return JSON schema for frontend UI generation

   **Request:** None

   **Response (200 OK):**

   .. code-block:: json

      {
        "title": "ESP32 Device Configuration",
        "version": "1.0.0",
        "fields": [
          {
            "key": "wifi_ssid",
            "type": "string",
            "label": "WiFi Network Name",
            "default": "",
            "required": false,
            "maxLength": 63,
            "group": "wifi"
          },
          {
            "key": "ap_ssid", 
            "type": "string",
            "label": "AP SSID",
            "default": "ESP32-Setup",
            "required": true,
            "maxLength": 32,
            "group": "ap"
          }
        ]
      }

   **Implementation:**

   .. code-block:: c

      static esp_err_t schema_get_handler(httpd_req_t *req)
      {
          httpd_resp_set_hdr(req, "Content-Type", "application/json");
          httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

          char *schema_json = NULL;
          esp_err_t ret = config_get_schema_json(&schema_json);
          if (ret != ESP_OK) {
              httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, 
                                  "Failed to load configuration schema");
              return ESP_FAIL;
          }

          httpd_resp_send(req, schema_json, HTTPD_RESP_USE_STRLEN);
          // Note: schema_json points to embedded flash data, no free() needed
          
          return ESP_OK;
      }

   ---

   **Endpoint: GET /api/config**

   **Purpose:** Retrieve all current configuration values

   **Request:** None

   **Response (200 OK):**

   .. code-block:: json

      {
        "wifi_ssid": "MyNetwork",
        "wifi_pass": "",
        "ap_ssid": "ESP32-Setup", 
        "ap_pass": "12345678",
        "led_count": 50,
        "led_bright": 128,
        "device_name": "MyDevice"
      }

   **Implementation:**

   .. code-block:: c

      static esp_err_t config_get_handler(httpd_req_t *req)
      {
          httpd_resp_set_hdr(req, "Content-Type", "application/json");
          httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

          char *config_json = NULL;
          esp_err_t ret = config_get_all_as_json(&config_json);
          if (ret != ESP_OK) {
              httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, 
                                  "Failed to read configuration");
              return ESP_FAIL;
          }

          httpd_resp_send(req, config_json, HTTPD_RESP_USE_STRLEN);
          free(config_json);
          
          return ESP_OK;
      }

   ---

   **Endpoint: POST /api/config**

   **Purpose:** Update configuration values

   **Request Body:**

   .. code-block:: json

      {
        "wifi_ssid": "NewNetwork",
        "wifi_pass": "newpassword123",
        "ap_ssid": "MyDevice-AP",
        "led_count": 144,
        "device_name": "UpdatedDevice"
      }

   **Response (200 OK):**

   .. code-block:: json

      {
        "status": "success",
        "message": "Configuration saved successfully"
      }

   **Implementation:**

   .. code-block:: c

      static esp_err_t config_set_handler(httpd_req_t *req)
      {
          httpd_resp_set_hdr(req, "Content-Type", "application/json");
          httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

          // Read request body
          char content[1024];
          int ret = httpd_req_recv(req, content, sizeof(content) - 1);
          if (ret <= 0) {
              httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request body");
              return ESP_FAIL;
          }
          content[ret] = '\0';

          esp_err_t config_ret = config_set_all_from_json(content);
          if (config_ret != ESP_OK) {
              httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Configuration update failed");
              return ESP_FAIL;
          }

          const char *response = "{\"status\":\"success\",\"message\":\"Configuration saved successfully\"}";
          httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
          
          return ESP_OK;
      }

   ---

   **Endpoint: POST /api/config/reset**

   **Purpose:** Reset all configuration to factory defaults

   **Request:** None

   **Response (200 OK):**

   .. code-block:: json

      {
        "status": "success",
        "message": "Configuration reset to factory defaults"
      }

   **Implementation:**

   .. code-block:: c

      static esp_err_t config_reset_handler(httpd_req_t *req)
      {
          httpd_resp_set_hdr(req, "Content-Type", "application/json");
          httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

          esp_err_t ret = config_factory_reset();
          if (ret != ESP_OK) {
              httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, 
                                  "Factory reset failed");
              return ESP_FAIL;
          }

          const char *response = "{\"status\":\"success\",\"message\":\"Configuration reset to factory defaults\"}";
          httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
          return ESP_OK;
      }


.. spec:: WiFi Management REST API Endpoints
   :id: SPEC_WEB_REST_WIFI_1
   :links: REQ_WEB_3
   :status: approved
   :tags: web, api, wifi

   **Endpoint: GET /scan**

   **Purpose:** Scan for available WiFi networks

   **Response:**

   .. code-block:: json

      {
        "networks": [
          {
            "ssid": "NetworkName",
            "rssi": -45,
            "authmode": 3
          }
        ]
      }

   **Implementation Notes:**

   - Automatically switches to APSTA mode if in AP mode (scanning requires STA)
   - Returns sorted list by signal strength (RSSI)
   - ``authmode``: 0=Open, 2=WPA-PSK, 3=WPA2-PSK, 4=WPA/WPA2-PSK

   ---

   **Endpoint: POST /connect**

   **Purpose:** Connect to WiFi network with provided credentials

   **Request:**

   .. code-block:: json

      {
        "ssid": "NetworkName",
        "password": "secretpassword"
      }

   **Response (200 OK):**

   .. code-block:: json

      {
        "success": true
      }

   **Integration:** Calls ``wifi_manager_set_credentials()`` to save credentials to NVS and initiate connection.

   ---

   **Endpoint: GET /status**

   **Purpose:** Get current WiFi connection status

   **Response:**

   .. code-block:: json

      {
        "mode": 1,
        "ssid": "ConnectedNetwork",
        "rssi": -52,
        "has_credentials": true,
        "ip": "192.168.1.100"
      }

   **Integration:** Calls ``wifi_manager_get_status()`` and ``wifi_manager_get_ip_address()``.

   ---

   **Endpoint: POST /reset**

   **Purpose:** Clear WiFi credentials and restart device in AP mode

   **Response:**

   .. code-block:: json

      {
        "success": true,
        "message": "Device will restart in AP mode in 3 seconds"
      }

   **Implementation:** Calls ``wifi_manager_clear_credentials()`` and schedules device restart.


.. spec:: System Health API Endpoint
   :id: SPEC_WEB_REST_HEALTH_1
   :links: REQ_WEB_1
   :status: approved
   :tags: web, api, diagnostics

   **Endpoint: GET /api/system/health**

   **Purpose:** System diagnostics and health monitoring

   **Response:**

   .. code-block:: json

      {
        "uptime_seconds": 3542.5,
        "free_heap_bytes": 125432,
        "minimum_free_heap_bytes": 98234,
        "heap_fragmentation_percent": 21.7,
        "configuration": {
          "status": "healthy",
          "api_version": "2.0"
        },
        "wifi": {
          "status": "connected",
          "ssid": "MyNetwork",
          "rssi": -48
        },
        "overall_status": "healthy",
        "device_type": "ESP32 Template",
        "firmware_version": "1.0.0"
      }

   **Health Assessment Logic:**

   .. code-block:: c

      bool system_healthy = (wifi_status == ESP_OK) && 
                           (free_heap > 50000); // At least 50KB free

   **Use Cases:**

   - Dashboard real-time monitoring
   - Remote diagnostics
   - Automated health checks


.. spec:: LED State API Endpoint
   :id: SPEC_WEB_REST_LED_1
   :links: REQ_WEB_LED_1, SPEC_LED_API_3
   :status: approved
   :tags: web, api, led, visualization

   **Endpoint: GET /api/led/state**

   **Purpose:** Retrieve current LED strip state for real-time web visualization

   **Response:**

   .. code-block:: json

      {
        "led_count": 40,
        "colors": "FF0000 00FF00 0000FF 000000 FFFFFF ..."
      }

   **Response Format:**

   - ``led_count``: Number of LEDs in the strip (matches configuration)
   - ``colors``: Space-separated hex RGB colors (e.g., "FF0000" = red, "000000" = off)
   - Each color: 6 hex digits (RRGGBB format)
   - Total response size: ~280 bytes for 40 LEDs + JSON overhead

   **Implementation Details:**

   .. code-block:: c

      // Handler function
      static esp_err_t led_state_handler(httpd_req_t *req)
      {
          // 1. Set CORS headers for cross-origin access
          httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
          httpd_resp_set_hdr(req, "Content-Type", "application/json");

          // 2. Get LED count from controller
          uint16_t led_count = led_get_count();
          
          // 3. Allocate snapshot buffer
          led_color_t *snapshot = malloc(led_count * sizeof(led_color_t));
          
          // 4. Get thread-safe snapshot using REQ_LED_5 API
          uint16_t actual = led_get_all_colors(snapshot, led_count);
          
          // 5. Convert RGB to hex string: "RRGGBB RRGGBB ..."
          //    Format: sprintf(pos, "%02X%02X%02X", r, g, b)
          
          // 6. Build JSON response
          // 7. Send response
          // 8. Free buffers
      }

   **Error Handling:**

   - LED controller not initialized → 500 Internal Server Error
   - Memory allocation failure → 500 Internal Server Error
   - Snapshot read failure → 500 Internal Server Error
   - All errors log to console for debugging

   **Performance:**

   - Typical response time: <100ms for 40 LEDs
   - Memory usage: ~600 bytes (LED snapshot + hex string buffers)
   - Thread-safe: Uses mutex-protected ``led_get_all_colors()``
   - No blocking of LED updates: Snapshot operation is fast

   **Use Cases:**

   - Real-time LED strip visualization in web UI
   - Remote monitoring of LED states
   - JavaScript canvas/CSS LED animation sync
   - Debugging LED display issues

   **CORS Support:**

   - Access-Control-Allow-Origin: * (allows cross-origin requests)
   - Enables local development and external web apps


Configuration Manager Integration
----------------------------------

.. spec:: Config Manager Integration Pattern
   :id: SPEC_WEB_INTEGRATION_CFG_1
   :links: REQ_WEB_CONF_1, REQ_CFG_JSON_10
   :status: approved
   :tags: web, integration, config

   **Integration Points:**

   The web server integrates with the config manager component through a well-defined C API:

   **Include Header:**

   .. code-block:: c

      #include "config_manager.h"  // Provides config_get_*, config_set_* functions

   **Reading Configuration:**

   .. code-block:: c

      char wifi_ssid[CONFIG_STRING_MAX_LEN + 1];
      uint16_t led_count;
      
      esp_err_t ret = config_get_string(CONFIG_WIFI_SSID, wifi_ssid, sizeof(wifi_ssid));
      if (ret != ESP_OK) {
          ESP_LOGE(TAG, "Failed to get WiFi SSID: %s", esp_err_to_name(ret));
          // Use default or return error to client
      }
      
      config_get_uint16(CONFIG_LED_COUNT, &led_count);

   **Writing Configuration:**

   .. code-block:: c

      const char *new_ssid = "NewNetwork";
      esp_err_t ret = config_set_string(CONFIG_WIFI_SSID, new_ssid);
      if (ret != ESP_OK) {
          ESP_LOGE(TAG, "Failed to set WiFi SSID: %s", esp_err_to_name(ret));
          httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid WiFi SSID");
          return ESP_FAIL;
      }

   **Factory Reset:**

   .. code-block:: c

      esp_err_t ret = config_factory_reset();
      if (ret != ESP_OK) {
          httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Factory reset failed");
          return ESP_FAIL;
      }

   **Error Code Mapping:**

   ============================  =================================  ===============
   Config Manager Error          HTTP Status Code                   Response Action
   ============================  =================================  ===============
   ``ESP_OK``                    200 OK                             Return data/success
   ``ESP_ERR_INVALID_ARG``       400 Bad Request                    Invalid parameter
   ``ESP_ERR_NVS_NOT_FOUND``     404 Not Found                      Key doesn't exist
   ``ESP_ERR_NO_MEM``            500 Internal Server Error          Memory allocation failed
   ``ESP_FAIL`` (NVS error)      500 Internal Server Error          Storage error
   ============================  =================================  ===============

   **Separation of Concerns:**

   - **Config Manager** owns NVS storage logic, key-value mapping, defaults
   - **Web Server** owns HTTP protocol, JSON serialization, client interaction
   - **Interface:** Simple C function calls with ``esp_err_t`` return codes

   **Note:** Config Manager does NOT know about HTTP, JSON, or web interfaces. Web Server does NOT know about NVS internals or storage keys.


WiFi Manager Integration
-------------------------

.. spec:: WiFi Manager Integration Pattern
   :id: SPEC_WEB_INTEGRATION_WIFI_1
   :links: REQ_WEB_3
   :status: approved
   :tags: web, integration, wifi

   **Integration Points:**

   .. code-block:: c

      #include "wifi_manager.h"

   **WiFi Scanning:**

   .. code-block:: c

      // Start scan (blocking)
      wifi_scan_config_t scan_config = {
          .show_hidden = false,
          .scan_type = WIFI_SCAN_TYPE_ACTIVE,
          .scan_time.active.min = 100,
          .scan_time.active.max = 300,
      };
      esp_wifi_scan_start(&scan_config, true);

      // Get results
      uint16_t ap_count = 0;
      esp_wifi_scan_get_ap_num(&ap_count);
      wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
      esp_wifi_scan_get_ap_records(&ap_count, ap_records);

   **Set WiFi Credentials:**

   .. code-block:: c

      wifi_credentials_t credentials = {0};
      strncpy(credentials.ssid, "MyNetwork", sizeof(credentials.ssid) - 1);
      strncpy(credentials.password, "password", sizeof(credentials.password) - 1);
      
      esp_err_t ret = wifi_manager_set_credentials(&credentials);

   **Get WiFi Status:**

   .. code-block:: c

      wifi_status_t status;
      wifi_manager_get_status(&status);
      
      // status.mode, status.connected_ssid, status.rssi, status.has_credentials

   **Clear Credentials:**

   .. code-block:: c

      esp_err_t ret = wifi_manager_clear_credentials();

   **Lifecycle:**

   - WiFi Manager starts web server when entering AP mode (captive portal)
   - WiFi Manager stops web server when successfully connecting to WiFi (optional behavior)
   - Web server can run independently in both AP and STA modes


Captive Portal Implementation
------------------------------

.. spec:: Captive Portal Design
   :id: SPEC_WEB_CAPTIVE_1
   :links: REQ_WEB_3
   :status: approved
   :tags: web, captive-portal, wifi

   **Current Implementation:**

   The template provides a **simplified captive portal** approach:

   1. **AP Mode:** Device starts in AP mode (``ESP32-AP``) if no WiFi credentials stored
   2. **Direct IP Access:** Users connect to AP and navigate to device IP (e.g., ``192.168.4.1``)
   3. **WiFi Setup Page:** ``/wifi-setup.html`` provides network scanning and credential entry
   4. **Manual Navigation:** No DNS redirect - users manually enter IP address

   **Previous DNS Server Approach (Removed for Simplicity):**

   Earlier versions included a DNS server to redirect all DNS queries to device IP for automatic captive portal popup. This was removed to simplify the template.

   **Re-enabling Captive Portal (Optional):**

   Users can optionally implement DNS redirect:

   1. Start DNS server in AP mode (listen UDP port 53)
   2. Respond to all DNS queries with device AP IP (``192.168.4.1``)
   3. Trigger browser captive portal detection (iOS, Android, Windows)

   **Current User Experience:**

   1. Connect to ``ESP32-AP`` WiFi network
   2. Open browser and navigate to ``http://192.168.4.1``
   3. Click "WiFi" in navigation menu
   4. Scan networks, select SSID, enter password, click "Connect"
   5. Device saves credentials and restarts in STA mode

   **Note:** This simplified approach works well for template usage. Production deployments may want full DNS redirect captive portal.


Server Configuration and Lifecycle
-----------------------------------

.. spec:: HTTP Server Configuration
   :id: SPEC_WEB_CONFIG_1
   :links: REQ_WEB_5
   :status: approved
   :tags: web, config, performance

   **Server Configuration:**

   .. code-block:: c

      typedef struct {
          uint16_t port;              // Default: 80
          uint8_t max_open_sockets;   // Default: 7
      } web_server_config_t;

      #define WEB_SERVER_DEFAULT_CONFIG() { \
          .port = 80, \
          .max_open_sockets = 7 \
      }

   **ESP-IDF httpd_config_t Settings:**

   .. code-block:: c

      httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
      httpd_config.server_port = 80;
      httpd_config.max_open_sockets = 7;     // Concurrent connections
      httpd_config.max_uri_handlers = 32;    // Increased from default 8
      httpd_config.lru_purge_enable = true;  // Auto-remove LRU handlers

   **Performance Characteristics:**

   - **Max Concurrent Connections:** 7 (ESP32 memory constraint)
   - **Connection Timeout:** 20 seconds default (ESP-IDF)
   - **Non-blocking I/O:** Event-driven, doesn't block FreeRTOS tasks
   - **Memory per Connection:** ~1-2KB depending on request size

   **Initialization Sequence:**

   .. code-block:: c

      esp_err_t web_server_init(const web_server_config_t *config)
      {
          // 1. Store configuration
          current_config = *config;
          
          // 2. Start HTTP server
          httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
          httpd_config.server_port = current_config.port;
          httpd_config.max_open_sockets = current_config.max_open_sockets;
          httpd_config.max_uri_handlers = 32;
          httpd_config.lru_purge_enable = true;
          
          if (httpd_start(&server, &httpd_config) != ESP_OK) {
              return ESP_FAIL;
          }
          
          // 3. Register all URI handlers (32 handlers total)
          // ... (registration code)
          
          return ESP_OK;
      }

   **Start/Stop Functions:**

   .. code-block:: c

      esp_err_t web_server_start(void)
      {
          server_running = true;
          ESP_LOGI(TAG, "Web server started successfully");
          return ESP_OK;
      }

      esp_err_t web_server_stop(void)
      {
          if (server != NULL) {
              httpd_stop(server);
              server = NULL;
          }
          server_running = false;
          return ESP_OK;
      }

   **Query Functions:**

   .. code-block:: c

      bool web_server_is_running(void);      // Check running state
      uint16_t web_server_get_port(void);    // Get configured port


Adding New Pages and Endpoints
-------------------------------

.. spec:: Extension Guide for Web Pages
   :id: SPEC_WEB_EXTEND_1
   :links: REQ_WEB_4
   :status: approved
   :tags: web, extensibility, guide

   **Adding a New Static HTML Page:**

   1. **Create HTML File:**

      Create ``main/components/web_server/www/my-page.html``

   2. **Update CMakeLists.txt:**

      .. code-block:: cmake

         EMBED_FILES 
             "www/index.html"
             "www/wifi-setup.html" 
             "www/settings.html"
             "www/my-page.html"   # ADD THIS LINE

   3. **Add Binary Symbol Declarations:**

      In ``web_server.c``:

      .. code-block:: c

         extern const uint8_t my_page_html_start[] asm("_binary_my_page_html_start");
         extern const uint8_t my_page_html_end[] asm("_binary_my_page_html_end");

   4. **Update ``get_embedded_file()``:**

      .. code-block:: c

         else if (strcmp(clean_filename, "/my-page.html") == 0)
         {
             *data = my_page_html_start;
             *size = my_page_html_end - my_page_html_start;
         }

   5. **Register URI Handler:**

      .. code-block:: c

         httpd_uri_t my_page_uri = {
             .uri = "/my-page.html",
             .method = HTTP_GET,
             .handler = static_file_handler,
             .user_ctx = NULL
         };
         httpd_register_uri_handler(server, &my_page_uri);

   6. **Add Navigation Link:**

      Update navbar in all HTML files:

      .. code-block:: html

         <a href="/my-page.html" class="nav-btn">My Page</a>

   **Adding a New REST API Endpoint:**

   1. **Implement Handler Function:**

      .. code-block:: c

         static esp_err_t my_api_handler(httpd_req_t *req)
         {
             httpd_resp_set_type(req, "application/json");
             httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
             
             // Your API logic here
             
             const char *response = "{\"status\":\"success\"}";
             return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
         }

   2. **Register URI Handler:**

      .. code-block:: c

         httpd_uri_t my_api_uri = {
             .uri = "/api/my-endpoint",
             .method = HTTP_GET,  // or HTTP_POST
             .handler = my_api_handler,
             .user_ctx = NULL
         };
         httpd_register_uri_handler(server, &my_api_uri);

   3. **Update JavaScript:**

      Add API call in ``www/js/app.js``:

      .. code-block:: javascript

         async function fetchMyData() {
             const response = await fetch('/api/my-endpoint');
             const data = await response.json();
             // Handle data
         }


CORS and Security
-----------------

.. spec:: CORS Configuration
   :id: SPEC_WEB_SECURITY_1
   :links: REQ_WEB_5
   :status: approved
   :tags: web, security, cors

   **Current CORS Policy:**

   All API endpoints return:

   .. code-block:: c

      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

   **Rationale:** Template simplicity - allows development from any origin (GitHub Pages, local files, etc.)

   **Production Recommendation:**

   For production deployments, restrict CORS to specific origins:

   .. code-block:: c

      // Option 1: Lock to device IP only (no external access)
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "null");
      
      // Option 2: Allow specific external domain (hybrid GitHub Pages approach)
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", 
                        "https://yourusername.github.io");

   **CORS Preflight Handler:**

   .. code-block:: c

      static esp_err_t cors_preflight_handler(httpd_req_t *req)
      {
          httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
          httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
          httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
          httpd_resp_set_hdr(req, "Access-Control-Max-Age", "86400"); // 24 hours
          httpd_resp_send(req, "", 0);
          return ESP_OK;
      }

   Registered for:

   .. code-block:: c

      httpd_uri_t options_uri = {
          .uri = "/api/*",    // Wildcard for all API endpoints
          .method = HTTP_OPTIONS,
          .handler = cors_preflight_handler,
          .user_ctx = NULL
      };

   **Password Exposure Protection:**

   Configuration GET endpoints never return password fields:

   .. code-block:: c

      cJSON_AddStringToObject(wifi, "password", "");  // Always empty string

   Passwords are only accepted in POST requests, never echoed back.


Testing and Debugging
----------------------

.. spec:: Web Server Testing Strategy
   :id: SPEC_WEB_TEST_1
   :links: REQ_WEB_5
   :status: approved
   :tags: web, testing

   **Manual Testing:**

   1. **QEMU Testing (No Hardware):**

      - Build and run in QEMU emulator
      - HTTP proxy required for browser access: ``python tools/http_proxy.py``
      - Access via ``http://localhost:8000``

   2. **Hardware Testing:**

      - Flash to ESP32 device
      - Connect to ``ESP32-AP`` WiFi network
      - Navigate to ``http://192.168.4.1``

   **Browser Developer Tools:**

   - **Network Tab:** Monitor HTTP requests/responses, status codes
   - **Console Tab:** Check for JavaScript errors
   - **Application Tab:** Inspect localStorage/sessionStorage

   **ESP32 Serial Monitor:**

   .. code-block:: text

      I (1234) web_server: Initializing web server on port 80
      I (1235) web_server: Registered handler for '/' - OK
      I (1236) web_server: Registered handler for '/scan' - OK
      I (1237) web_server: Web server initialized successfully
      I (5678) web_server: Serving static file: /index.html
      I (5679) web_server: Found index.html, size: 4823

   **Common Issues:**

   1. **404 Not Found:** Check ``get_embedded_file()`` has correct path mapping
   2. **Empty Response:** Verify EMBED_FILES in CMakeLists.txt
   3. **CORS Errors:** Ensure ``Access-Control-Allow-Origin`` header set
   4. **Memory Errors:** Reduce ``max_open_sockets`` or check heap usage


Traceability
------------

All traceability is automatically generated by Sphinx-Needs based on the ``:links:`` attributes in each specification.

.. needtable::
   :columns: id, title, status, tags

.. needflow:: SPEC_WEB_ARCH_1
