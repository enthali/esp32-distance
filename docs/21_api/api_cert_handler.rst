Certificate Handler API
======================

.. apicomponent:: Certificate Handler
   :id: API_COMP_CERT_HANDLER
   :status: implemented
   :header_file: main/components/cert_handler/cert_handler.h
   :links: REQ_SYS_SEC_1

   **Brief Description**

   Certificate management for ESP32 HTTPS server. Provides access to embedded SSL certificates generated during the build process, supporting long-term IoT deployments.

   Key features:

   * Embedded server and CA certificates
   * 25-year validity for IoT deployments
   * Build-time certificate generation

   **Architecture:**

   - Certificates generated at build time and embedded in firmware
   - Accessed via type-safe API for HTTPS server initialization

Lifecycle Functions
-------------------

.. apifunction:: cert_handler_init
   :id: API_FUNC_CERT_HANDLER_INIT
   :status: implemented
   :component: API_COMP_CERT_HANDLER
   :api_signature: esp_err_t cert_handler_init(void)
   :returns: ESP_OK if all certificates are available, error codes otherwise
   :parameters: None
   :links: REQ_SYS_SEC_1

   Initialize certificate management. Verifies that all required certificates are available and properly embedded in the firmware. Should be called during system initialization before starting the HTTPS server.

   **Signature:**

   .. code-block:: c

      esp_err_t cert_handler_init(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - All certificates are available
   * ``ESP_ERR_NOT_FOUND`` - Any certificates are missing

   .. note::
      Call before HTTPS server startup.

Certificate Access Functions
---------------------------

.. apifunction:: cert_handler_get_server_cert
   :id: API_FUNC_CERT_HANDLER_GET_SERVER_CERT
   :status: implemented
   :component: API_COMP_CERT_HANDLER
   :api_signature: esp_err_t cert_handler_get_server_cert(const char** cert_data, size_t* cert_len)
   :returns: ESP_OK on success, error codes otherwise
   :parameters: cert_data, cert_len
   :links: REQ_SYS_SEC_1

   Get embedded server certificate data (PEM format) generated during build.

   **Signature:**

   .. code-block:: c

      esp_err_t cert_handler_get_server_cert(const char** cert_data, size_t* cert_len);

   **Parameters:**

   * ``cert_data`` - Pointer to certificate data (null-terminated)
   * ``cert_len`` - Length of certificate data including null terminator

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - Parameters are NULL

.. apifunction:: cert_handler_get_server_key
   :id: API_FUNC_CERT_HANDLER_GET_SERVER_KEY
   :status: implemented
   :component: API_COMP_CERT_HANDLER
   :api_signature: esp_err_t cert_handler_get_server_key(const char** key_data, size_t* key_len)
   :returns: ESP_OK on success, error codes otherwise
   :parameters: key_data, key_len
   :links: REQ_SYS_SEC_1

   Get embedded server private key data (PEM format) generated during build.

   **Signature:**

   .. code-block:: c

      esp_err_t cert_handler_get_server_key(const char** key_data, size_t* key_len);

   **Parameters:**

   * ``key_data`` - Pointer to private key data (null-terminated)
   * ``key_len`` - Length of private key data including null terminator

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - Parameters are NULL

.. apifunction:: cert_handler_get_ca_cert
   :id: API_FUNC_CERT_HANDLER_GET_CA_CERT
   :status: implemented
   :component: API_COMP_CERT_HANDLER
   :api_signature: esp_err_t cert_handler_get_ca_cert(const char** cert_data, size_t* cert_len)
   :returns: ESP_OK on success, error codes otherwise
   :parameters: cert_data, cert_len
   :links: REQ_SYS_SEC_1

   Get embedded CA certificate data (PEM format) generated during build.

   **Signature:**

   .. code-block:: c

      esp_err_t cert_handler_get_ca_cert(const char** cert_data, size_t* cert_len);

   **Parameters:**

   * ``cert_data`` - Pointer to CA certificate data (null-terminated)
   * ``cert_len`` - Length of CA certificate data including null terminator

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - Parameters are NULL

.. apifunction:: cert_handler_get_info
   :id: API_FUNC_CERT_HANDLER_GET_INFO
   :status: implemented
   :component: API_COMP_CERT_HANDLER
   :api_signature: esp_err_t cert_handler_get_info(char* info_buffer, size_t buffer_size)
   :returns: ESP_OK on success, error codes otherwise
   :parameters: info_buffer, buffer_size
   :links: REQ_SYS_SEC_1

   Get certificate information for logging. Provides certificate metadata for diagnostics. Does not expose sensitive private key information.

   **Signature:**

   .. code-block:: c

      esp_err_t cert_handler_get_info(char* info_buffer, size_t buffer_size);

   **Parameters:**

   * ``info_buffer`` - Buffer to store certificate information
   * ``buffer_size`` - Size of the information buffer

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_SIZE`` - Buffer too small
