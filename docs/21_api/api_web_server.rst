Web Server API
==============

.. apicomponent:: Web Server
   :id: API_COMP_WEB_SERVER
   :status: implemented
   :header_file: main/components/web_server/web_server.h
   :links: REQ_SYS_WEB_1, REQ_WEB_1, REQ_WEB_2, REQ_WEB_3, REQ_WEB_CONF_1, REQ_WEB_SCHEMA_1, REQ_WEB_4, REQ_WEB_5, REQ_WEB_LED_1, REQ_SYS_NET_1, REQ_SYS_CFG_1, REQ_SYS_ARCH_1, SPEC_WEB_SERVER, SPEC_CONFIG_WEBPAGE
   
   HTTP server for WiFi captive portal and configuration interface.

   Key features:
   
   * Multi-page responsive web interface with navbar navigation
   * Static file serving from embedded flash assets (HTML, CSS, JS)
   * WiFi configuration API endpoints (scan, connect, status, reset)
   * Integration with DNS server module for captive portal functionality
   * CORS-secured API endpoints with proper MIME type handling

   **Architecture:**
   Implements HTTP server and DNS server for captive portal (AP mode). Serves static web assets from flash. Provides REST API endpoints for configuration, WiFi setup, and LED state. Integrates with config_manager and wifi_manager. Designed for modular, component-based ESP-IDF architecture.

Lifecycle Functions
------------------

.. apifunction:: web_server_init
   :id: API_FUNC_WEB_INIT
   :status: implemented
   :component: API_COMP_WEB_SERVER
   :api_signature: esp_err_t web_server_init(const web_server_config_t *config)
   :returns: ESP_OK on success, error code on failure
   :parameters: config (server configuration or NULL)
   :links: REQ_SYS_WEB_1, REQ_WEB_1, REQ_WEB_2, REQ_WEB_3, REQ_WEB_CONF_1, REQ_WEB_4, REQ_WEB_5, SPEC_WEB_SERVER

   Initialize web server with embedded static assets.

   **Signature:**

   .. code-block:: c

      esp_err_t web_server_init(const web_server_config_t *config);

   **Parameters:**

   * ``config`` - Server configuration (NULL for default settings)

   **Returns:**

   * ``ESP_OK`` - Initialization successful
   * ``ESP_ERR_NO_MEM`` - Out of memory
   * ``ESP_ERR_INVALID_ARG`` - Invalid configuration
   * ``ESP_FAIL`` - Other errors

   .. note::
      Sets up all URI handlers for static files and API endpoints.

.. apifunction:: web_server_start
   :id: API_FUNC_WEB_START
   :status: implemented
   :component: API_COMP_WEB_SERVER
   :api_signature: esp_err_t web_server_start(void)
   :returns: ESP_OK on success, error code on failure
   :parameters: None
   :links: REQ_WEB_3, REQ_SYS_NET_1, SPEC_WEB_SERVER

   Start web server and DNS server for captive portal.

   **Signature:**

   .. code-block:: c

      esp_err_t web_server_start(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Server started successfully
   * ``ESP_ERR_INVALID_STATE`` - Server already running
   * ``ESP_FAIL`` - Other errors

   .. note::
      Starts DNS server for captive portal in AP mode.

.. apifunction:: web_server_stop
   :id: API_FUNC_WEB_STOP
   :status: implemented
   :component: API_COMP_WEB_SERVER
   :api_signature: esp_err_t web_server_stop(void)
   :returns: ESP_OK on success, error code on failure
   :parameters: None
   :links: REQ_WEB_5, SPEC_WEB_SERVER

   Stop web server and DNS server.

   **Signature:**

   .. code-block:: c

      esp_err_t web_server_stop(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Server stopped successfully
   * ``ESP_ERR_INVALID_STATE`` - Server not running
   * ``ESP_FAIL`` - Other errors

   .. note::
      Safe to call multiple times or when servers are not running.

.. apifunction:: web_server_is_running
   :id: API_FUNC_WEB_IS_RUNNING
   :status: implemented
   :component: API_COMP_WEB_SERVER
   :api_signature: bool web_server_is_running(void)
   :returns: true if HTTP server is active, false otherwise
   :parameters: None
   :links: REQ_WEB_5

   Check if web server is currently running.

   **Signature:**

   .. code-block:: c

      bool web_server_is_running(void);

   **Parameters:**

   * None

   **Returns:**

   * ``true`` - HTTP server is active
   * ``false`` - HTTP server is not running

.. apifunction:: web_server_get_port
   :id: API_FUNC_WEB_GET_PORT
   :status: implemented
   :component: API_COMP_WEB_SERVER
   :api_signature: uint16_t web_server_get_port(void)
   :returns: Server port number
   :parameters: None
   :links: REQ_WEB_5

   Get the current HTTP server port number.

   **Signature:**

   .. code-block:: c

      uint16_t web_server_get_port(void);

   **Parameters:**

   * None

   **Returns:**

   * ``uint16_t`` - Server port number (typically 80)

Static File Handler
-------------------

.. apifunction:: static_file_handler
   :id: API_FUNC_WEB_STATIC_FILE
   :status: implemented
   :component: API_COMP_WEB_SERVER
   :api_signature: esp_err_t static_file_handler(httpd_req_t *req)
   :returns: ESP_OK on success, ESP_FAIL on file not found or other errors
   :parameters: req (HTTP request object)
   :links: REQ_WEB_1, REQ_WEB_2, REQ_WEB_4, REQ_WEB_SCHEMA_1, SPEC_CONFIG_WEBPAGE

   HTTP handler for serving embedded static files.

   **Signature:**

   .. code-block:: c

      esp_err_t static_file_handler(httpd_req_t *req);

   **Parameters:**

   * ``req`` - HTTP request object

   **Returns:**

   * ``ESP_OK`` - File served successfully
   * ``ESP_FAIL`` - File not found or other errors

   .. note::
      Serves HTML, CSS, and JS files from embedded flash. Sets MIME types and cache headers.

Data Types
----------

.. apistruct:: web_server_config_t
   :id: API_STRUCT_WEB_CONFIG
   :status: implemented
   :component: API_COMP_WEB_SERVER
   :links: REQ_WEB_5

   Web server configuration structure.

   **Definition:**

   .. code-block:: c

      typedef struct {
          uint16_t port;
          size_t max_open_sockets;
      } web_server_config_t;

   **Fields:**

   * ``port`` - HTTP server port (default: 80)
   * ``max_open_sockets`` - Maximum concurrent connections (default: 7)
