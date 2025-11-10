Network Tunnel API
==================

.. apicomponent:: Network Tunnel
   :id: API_COMP_NETIF_TUNNEL
   :status: implemented
   :header_file: main/components/netif_uart_tunnel/netif_uart_tunnel_sim.h
   :links: REQ_NETIF_TUNNEL_1, REQ_NETIF_TUNNEL_2, REQ_NETIF_TUNNEL_5

   **Brief Description**

   UART-based IP tunnel network interface for QEMU simulator. Provides a lwIP network interface that tunnels IP packets over UART1, enabling full TCP/IP stack functionality in QEMU without network device emulation.

   Key features:

   * QEMU UART network bridge for ESP32 emulation
   * lwIP network interface over UART1
   * Simple length-prefixed framing protocol
   * Conditional compilation for QEMU only

   **Architecture:**

   - ESP32 (QEMU): lwIP stack -> netif -> UART1 -> TCP socket
   - Host: TCP socket -> TUN device -> Linux network stack

Configuration Structure
-----------------------

.. apistruct:: netif_uart_tunnel_config_t
   :id: API_STRUCT_NETIF_TUNNEL_CONFIG
   :status: implemented
   :component: API_COMP_NETIF_TUNNEL
   :links: REQ_NETIF_TUNNEL_1

   Configuration for UART tunnel network interface.

   **Definition:**

   .. code-block:: c

      typedef struct {
          const char* hostname;
          uint8_t ip_addr[4];
          uint8_t netmask[4];
          uint8_t gateway[4];
      } netif_uart_tunnel_config_t;

   **Fields:**

   * ``hostname`` - Hostname for the interface
   * ``ip_addr`` - Static IP address (e.g., {192,168,100,2})
   * ``netmask`` - Netmask (e.g., {255,255,255,0})
   * ``gateway`` - Gateway IP (e.g., {192,168,100,1})

Lifecycle Functions
-------------------

.. apifunction:: netif_uart_tunnel_init
   :id: API_FUNC_NETIF_TUNNEL_INIT
   :status: implemented
   :component: API_COMP_NETIF_TUNNEL
   :api_signature: esp_err_t netif_uart_tunnel_init(const netif_uart_tunnel_config_t *config)
   :returns: ESP_OK on success, error codes on failure
   :parameters: config - network configuration
   :links: REQ_NETIF_TUNNEL_1

   Initialize UART tunnel network interface. Creates and configures a lwIP network interface that tunnels IP packets over UART1. Starts a background task to handle packet reception from UART.

   **Signature:**

   .. code-block:: c

      esp_err_t netif_uart_tunnel_init(const netif_uart_tunnel_config_t *config);

   **Parameters:**

   * ``config`` - Network configuration (IP, netmask, gateway)

   **Returns:**

   * ``ESP_OK`` - Initialization successful
   * ``ESP_ERR_INVALID_ARG`` - Config is NULL or contains invalid values
   * ``ESP_ERR_NO_MEM`` - Memory allocation failed
   * ``ESP_FAIL`` - UART or lwIP initialization failed

   .. note::
      Thread-safe. Only call once; subsequent calls return ESP_ERR_INVALID_STATE.

.. apifunction:: netif_uart_tunnel_deinit
   :id: API_FUNC_NETIF_TUNNEL_DEINIT
   :status: implemented
   :component: API_COMP_NETIF_TUNNEL
   :api_signature: esp_err_t netif_uart_tunnel_deinit(void)
   :returns: ESP_OK on success, error codes on failure
   :parameters: None
   :links: REQ_NETIF_TUNNEL_1

   Deinitialize UART tunnel network interface. Stops the receive task and cleans up resources. Active network connections will be closed by lwIP stack.

   **Signature:**

   .. code-block:: c

      esp_err_t netif_uart_tunnel_deinit(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Deinitialization successful
   * ``ESP_ERR_INVALID_STATE`` - Interface was not initialized
   * ``ESP_FAIL`` - Cleanup failed

   .. note::
      Thread-safe. Idempotent after first call. All active TCP/UDP connections will be terminated.

.. apifunction:: netif_uart_tunnel_get_handle
   :id: API_FUNC_NETIF_TUNNEL_GET_HANDLE
   :status: implemented
   :component: API_COMP_NETIF_TUNNEL
   :api_signature: esp_netif_t* netif_uart_tunnel_get_handle(void)
   :returns: esp_netif handle or NULL
   :parameters: None
   :links: REQ_NETIF_TUNNEL_1

   Get the esp_netif handle for the UART tunnel interface. Useful for registering event handlers or querying interface status.

   **Signature:**

   .. code-block:: c

      esp_netif_t* netif_uart_tunnel_get_handle(void);

   **Parameters:**

   * None

   **Returns:**

   * ``esp_netif_t*`` - Handle if interface is initialized
   * ``NULL`` - If not initialized

   .. note::
      Thread-safe. Handle remains valid until deinit is called.
