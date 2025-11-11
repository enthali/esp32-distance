WiFi Manager API
================

.. apicomponent:: WiFi Manager
   :id: API_COMP_WIFI_MGR
   :status: implemented
   :header_file: main/components/web_server/wifi_manager.h
   :links: REQ_SYS_NET_1, REQ_SYS_WEB_1, REQ_WEB_3, REQ_WEB_5, REQ_SYS_CFG_1, REQ_SYS_ARCH_1, SPEC_WEB_SERVER

   Smart WiFi management with captive portal and credential persistence.

   Key features:

   * Intelligent WiFi connection logic (AP/STA fallback)
   * Captive portal for WiFi setup
   * Credential storage in NVS
   * Integration with web_server and config_manager

   **Architecture:**
   Manages WiFi connection state machine (AP/STA/Disconnected). Provides captive portal for WiFi setup. Stores credentials in NVS and handles fallback logic. Integrates with web_server and config_manager. Designed for modular, component-based ESP-IDF architecture.

Lifecycle Functions
------------------

.. apifunction:: wifi_manager_init
   :id: API_FUNC_WIFI_INIT
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_init(void)
   :returns: ESP_OK on success, error code on failure
   :parameters: None
   :links: REQ_SYS_NET_1, REQ_WEB_3, SPEC_WEB_SERVER

   Initialize WiFi manager with smart boot logic.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_init(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Initialization successful
   * ``ESP_ERR_NO_MEM`` - Out of memory
   * ``ESP_ERR_FAIL`` - Other errors

   .. note::
      Automatically tries stored credentials, falls back to AP mode if needed.

.. apifunction:: wifi_manager_start
   :id: API_FUNC_WIFI_START
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_start(void)
   :returns: ESP_OK on success, error code on failure
   :parameters: None
   :links: REQ_SYS_NET_1, REQ_WEB_3

   Start WiFi manager (called after init).

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_start(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Started successfully
   * ``ESP_ERR_INVALID_STATE`` - Already started
   * ``ESP_FAIL`` - Other errors

.. apifunction:: wifi_manager_stop
   :id: API_FUNC_WIFI_STOP
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_stop(void)
   :returns: ESP_OK on success, error code on failure
   :parameters: None
   :links: REQ_SYS_NET_1

   Stop WiFi manager.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_stop(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Stopped successfully
   * ``ESP_ERR_INVALID_STATE`` - Not running
   * ``ESP_FAIL`` - Other errors

Configuration/Status Functions
------------------------------

.. apifunction:: wifi_manager_get_status
   :id: API_FUNC_WIFI_GET_STATUS
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_get_status(wifi_status_t *status)
   :returns: ESP_OK on success, ESP_ERR_INVALID_ARG if status is NULL
   :parameters: status (pointer to status structure)
   :links: REQ_SYS_NET_1, REQ_WEB_3

   Get current WiFi status.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_get_status(wifi_status_t *status);

   **Parameters:**

   * ``status`` - Pointer to status structure to fill

   **Returns:**

   * ``ESP_OK`` - Status filled successfully
   * ``ESP_ERR_INVALID_ARG`` - status is NULL
   * ``ESP_FAIL`` - Other errors

.. apifunction:: wifi_manager_set_credentials
   :id: API_FUNC_WIFI_SET_CRED
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_set_credentials(const wifi_credentials_t *credentials)
   :returns: ESP_OK on success, error code on failure
   :parameters: credentials (WiFi credentials to save and connect to)
   :links: REQ_WEB_3, REQ_SYS_CFG_1

   Save WiFi credentials and attempt connection.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_set_credentials(const wifi_credentials_t *credentials);

   **Parameters:**

   * ``credentials`` - WiFi credentials to save and connect to

   **Returns:**

   * ``ESP_OK`` - Credentials saved and connection attempted
   * ``ESP_ERR_INVALID_ARG`` - Invalid credentials
   * ``ESP_FAIL`` - Other errors

.. apifunction:: wifi_manager_clear_credentials
   :id: API_FUNC_WIFI_CLEAR_CRED
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_clear_credentials(void)
   :returns: ESP_OK on success, error code on failure
   :parameters: None
   :links: REQ_WEB_3, REQ_SYS_CFG_1

   Clear stored WiFi credentials.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_clear_credentials(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Credentials cleared and switched to AP mode
   * ``ESP_FAIL`` - Other errors

.. apifunction:: wifi_manager_get_ip_address
   :id: API_FUNC_WIFI_GET_IP
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_get_ip_address(char *ip_str, size_t max_len)
   :returns: ESP_OK on success, ESP_ERR_INVALID_STATE if no IP assigned
   :parameters: ip_str (buffer), max_len (buffer length)
   :links: REQ_SYS_NET_1

   Get current IP address as string.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_get_ip_address(char *ip_str, size_t max_len);

   **Parameters:**

   * ``ip_str`` - Buffer to store IP address string (min 16 bytes)
   * ``max_len`` - Maximum length of ip_str buffer

   **Returns:**

   * ``ESP_OK`` - IP address copied to buffer
   * ``ESP_ERR_INVALID_STATE`` - No IP assigned
   * ``ESP_FAIL`` - Other errors

Utility Functions
-----------------

.. apifunction:: wifi_manager_switch_to_ap
   :id: API_FUNC_WIFI_SWITCH_AP
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_switch_to_ap(void)
   :returns: ESP_OK on success, error code on failure
   :parameters: None
   :links: REQ_WEB_3, REQ_SYS_NET_1

   Force switch to AP mode.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_switch_to_ap(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Switched to AP mode
   * ``ESP_FAIL`` - Other errors

.. apifunction:: wifi_manager_monitor
   :id: API_FUNC_WIFI_MONITOR
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :api_signature: esp_err_t wifi_manager_monitor(void)
   :returns: ESP_OK on success
   :parameters: None
   :links: REQ_SYS_NET_1

   Perform lightweight WiFi health monitoring.

   **Signature:**

   .. code-block:: c

      esp_err_t wifi_manager_monitor(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Monitoring performed successfully
   * ``ESP_FAIL`` - Other errors

   .. note::
      Should be called periodically from main.c (at least once every 30 seconds).

Data Types
----------

.. apistruct:: wifi_manager_mode_t
   :id: API_STRUCT_WIFI_MODE
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :links: REQ_SYS_NET_1

   WiFi manager modes.

   **Definition:**

   .. code-block:: c

      typedef enum {
          WIFI_MODE_DISCONNECTED = 0,
          WIFI_MODE_STA_CONNECTING,
          WIFI_MODE_STA_CONNECTED,
          WIFI_MODE_AP_ACTIVE,
          WIFI_MODE_SWITCHING
      } wifi_manager_mode_t;

   **Values:**

   * ``WIFI_MODE_DISCONNECTED`` - Not connected to any network
   * ``WIFI_MODE_STA_CONNECTING`` - Attempting to connect to STA
   * ``WIFI_MODE_STA_CONNECTED`` - Connected to STA network
   * ``WIFI_MODE_AP_ACTIVE`` - AP mode active with captive portal
   * ``WIFI_MODE_SWITCHING`` - Transitioning between modes

.. apistruct:: wifi_credentials_t
   :id: API_STRUCT_WIFI_CRED
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :links: REQ_SYS_NET_1

   WiFi credentials structure.

   **Definition:**

   .. code-block:: c

      typedef struct {
          char ssid[32];
          char password[64];
      } wifi_credentials_t;

   **Fields:**

   * ``ssid`` - WiFi SSID (max 31 chars + null terminator)
   * ``password`` - WiFi password (max 63 chars + null terminator)

.. apistruct:: wifi_status_t
   :id: API_STRUCT_WIFI_STATUS
   :status: implemented
   :component: API_COMP_WIFI_MGR
   :links: REQ_SYS_NET_1

   WiFi status information.

   **Definition:**

   .. code-block:: c

      typedef struct {
          wifi_manager_mode_t mode;
          char connected_ssid[32];
          int8_t rssi;
          uint8_t retry_count;
          bool has_credentials;
      } wifi_status_t;

   **Fields:**

   * ``mode`` - Current WiFi mode
   * ``connected_ssid`` - Currently connected SSID (if any)
   * ``rssi`` - Signal strength (dBm)
   * ``retry_count`` - Current retry attempt
   * ``has_credentials`` - Whether stored credentials exist
