Config Manager API
===================

.. apicomponent:: Config Manager
   :id: API_COMP_CONFIG_MANAGER
   :status: implemented
   :header_file: main/components/config_manager/config_manager.h
   :links: REQ_CFG_JSON_1, REQ_CFG_JSON_4, REQ_CFG_JSON_6, REQ_CFG_JSON_7, REQ_CFG_JSON_8, REQ_CFG_JSON_9, REQ_CFG_JSON_11, REQ_CFG_JSON_12, REQ_CFG_JSON_13
   
   JSON Schema-driven configuration manager that stores individual
   configuration values in NVS using keys defined by `config_schema.json`.
   The component exposes type-safe getters/setters for string, int32,
   int16 and boolean values and provides bulk JSON import/export helpers
   used by the web UI.
   
   Key features:

   * JSON Schema as single source of truth (embedded at build time)
   * Direct key-based NVS storage for minimal RAM usage
   * Type-safe API for string, integer and boolean parameters
   * Bulk JSON read/write operations for UI integration
   
   **Architecture:**

   Thin wrapper around ESP-IDF NVS with no runtime validation. Validation
   is expected to be performed by the browser UI using the embedded JSON
   schema. Factory defaults are generated at build-time and written by an
   auto-generated helper.

Lifecycle Functions
-------------------

.. apifunction:: config_init
   :id: API_FUNC_CONFIG_INIT
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_init(void)
   :returns: Summary for metadata
   :parameters: None
   :links: REQ_CFG_JSON_12
   
   Initialize configuration manager by opening the NVS namespace "config".
   If the namespace is uninitialized, factory defaults are written.

   **Signature:**

   .. code-block:: c

      esp_err_t config_init(void)

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Initialization successful
   * ``ESP_ERR_NVS_*`` - NVS initialization or operation failure

   .. note::
      Must be called once during system startup. Implements REQ_CFG_JSON_12.

.. apifunction:: config_factory_reset
   :id: API_FUNC_CONFIG_FACTORY_RESET
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_factory_reset(void)
   :returns: Summary for metadata
   :parameters: None
   :links: REQ_CFG_JSON_9
   
   Reset all configuration to factory defaults by erasing the NVS
   namespace and writing generated defaults.

   **Signature:**

   .. code-block:: c

      esp_err_t config_factory_reset(void)

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_NVS_*`` - NVS operation failure

   .. note::
      Uses auto-generated factory write helper (see config_write_factory_defaults()).

.. apifunction:: config_commit
   :id: API_FUNC_CONFIG_COMMIT
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_commit(void)
   :returns: Summary for metadata
   :parameters: None
   
   Commit pending NVS writes to flash. Useful after multiple _no_commit
   updates to batch flash writes.

   **Signature:**

   .. code-block:: c

      esp_err_t config_commit(void)

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_NVS_*`` - Commit failure

   .. note::
      Use to reduce flash wear when performing multiple updates.

.. apifunction:: config_write_factory_defaults
   :id: API_FUNC_CONFIG_WRITE_FACTORY_DEFAULTS
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: void config_write_factory_defaults(void)
   :returns: Summary for metadata
   :parameters: None
   :links: REQ_CFG_JSON_4
   
   Auto-generated helper that writes factory default values to NVS. It is
   generated at build-time from `config_schema.json` and should not be
   called directly by applications; use ``config_factory_reset()`` instead.

   **Signature:**

   .. code-block:: c

      void config_write_factory_defaults(void)

   **Parameters:**

   * None

   **Returns:**

   * None

   .. note::
      Generated during build - reference only.

String Parameter Access
-----------------------

.. apifunction:: config_get_string
   :id: API_FUNC_CONFIG_GET_STRING
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_get_string(const char* key, char* buffer, size_t buf_len)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Read a null-terminated string value from NVS into a provided buffer.

   **Signature:**

   .. code-block:: c

      esp_err_t config_get_string(const char* key, char* buffer, size_t buf_len)

   **Parameters:**

   * ``key`` - NVS key name (must match schema field key)
   * ``buffer`` - Destination buffer to receive string (including null)
   * ``buf_len`` - Size of ``buffer`` in bytes

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` or ``buffer`` is NULL
   * ``ESP_ERR_NVS_NOT_FOUND`` - Key not found in NVS
   * ``ESP_ERR_NVS_INVALID_LENGTH`` - Provided buffer too small

   .. note::
      No runtime validation performed; the UI should enforce schema constraints.

.. apifunction:: config_set_string
   :id: API_FUNC_CONFIG_SET_STRING
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_string(const char* key, const char* value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Write a null-terminated string value to NVS and commit immediately.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_string(const char* key, const char* value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - Null-terminated string value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` or ``value`` is NULL
   * ``ESP_ERR_NVS_*`` - NVS write failure

.. apifunction:: config_set_string_no_commit
   :id: API_FUNC_CONFIG_SET_STRING_NO_COMMIT
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_string_no_commit(const char* key, const char* value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   
   Write a string to NVS without committing. Call ``config_commit()`` to
   persist batched updates.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_string_no_commit(const char* key, const char* value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - Null-terminated string value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` or ``value`` is NULL
   * ``ESP_ERR_NVS_*`` - NVS write failure

Integer Parameter Access
------------------------

.. apifunction:: config_get_int32
   :id: API_FUNC_CONFIG_GET_INT32
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_get_int32(const char* key, int32_t* value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Read a 32-bit signed integer from NVS.

   **Signature:**

   .. code-block:: c

      esp_err_t config_get_int32(const char* key, int32_t* value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - Pointer to store retrieved value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` or ``value`` is NULL
   * ``ESP_ERR_NVS_NOT_FOUND`` - Key not found

.. apifunction:: config_set_int32
   :id: API_FUNC_CONFIG_SET_INT32
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_int32(const char* key, int32_t value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Write a 32-bit signed integer to NVS and commit immediately.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_int32(const char* key, int32_t value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - New parameter value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` is NULL
   * ``ESP_ERR_NVS_*`` - NVS write failure

.. apifunction:: config_get_int16
   :id: API_FUNC_CONFIG_GET_INT16
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_get_int16(const char* key, int16_t* value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Read a 16-bit signed integer from NVS.

   **Signature:**

   .. code-block:: c

      esp_err_t config_get_int16(const char* key, int16_t* value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - Pointer to store retrieved value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` or ``value`` is NULL
   * ``ESP_ERR_NVS_NOT_FOUND`` - Key not found

.. apifunction:: config_set_int16
   :id: API_FUNC_CONFIG_SET_INT16
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_int16(const char* key, int16_t value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Write a 16-bit signed integer to NVS and commit immediately.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_int16(const char* key, int16_t value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - New parameter value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` is NULL
   * ``ESP_ERR_NVS_*`` - NVS write failure

.. apifunction:: config_set_int16_no_commit
   :id: API_FUNC_CONFIG_SET_INT16_NO_COMMIT
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_int16_no_commit(const char* key, int16_t value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   
   Write an int16 value to NVS without committing; use ``config_commit()`` to
   persist batched updates.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_int16_no_commit(const char* key, int16_t value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - New parameter value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` is NULL
   * ``ESP_ERR_NVS_*`` - NVS write failure

Boolean Parameter Access
------------------------

.. apifunction:: config_get_bool
   :id: API_FUNC_CONFIG_GET_BOOL
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_get_bool(const char* key, bool* value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Read a boolean value (stored as uint8) from NVS.

   **Signature:**

   .. code-block:: c

      esp_err_t config_get_bool(const char* key, bool* value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - Pointer to store result

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` or ``value`` is NULL
   * ``ESP_ERR_NVS_NOT_FOUND`` - Key not found

.. apifunction:: config_set_bool_no_commit
   :id: API_FUNC_CONFIG_SET_BOOL_NO_COMMIT
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_bool_no_commit(const char* key, bool value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   
   Write a boolean value to NVS without committing; use ``config_commit()``
   to persist batched updates.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_bool_no_commit(const char* key, bool value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - New boolean value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` is NULL
   * ``ESP_ERR_NVS_*`` - NVS write failure

.. apifunction:: config_set_bool
   :id: API_FUNC_CONFIG_SET_BOOL
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_bool(const char* key, bool value)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_7
   
   Write a boolean value to NVS and commit immediately.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_bool(const char* key, bool value)

   **Parameters:**

   * ``key`` - NVS key name
   * ``value`` - New boolean value

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - ``key`` is NULL
   * ``ESP_ERR_NVS_*`` - NVS write failure

Bulk JSON Configuration API
---------------------------

.. apifunction:: config_get_schema_json
   :id: API_FUNC_CONFIG_GET_SCHEMA_JSON
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_get_schema_json(char **schema_json)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_12
   
   Return pointer to embedded JSON schema string. The returned pointer is
   owned by the binary and must not be freed by the caller.

   **Signature:**

   .. code-block:: c

      esp_err_t config_get_schema_json(char **schema_json)

   **Parameters:**

   * ``schema_json`` - Output pointer to embedded schema string

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_NOT_FOUND`` - Schema not embedded
   * ``ESP_ERR_INVALID_ARG`` - ``schema_json`` is NULL

.. apifunction:: config_get_all_as_json
   :id: API_FUNC_CONFIG_GET_ALL_AS_JSON
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_get_all_as_json(char **config_json)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_12
   
   Build and return allocated JSON array string representing all
   configuration values. Caller must free the returned string.

   **Signature:**

   .. code-block:: c

      esp_err_t config_get_all_as_json(char **config_json)

   **Parameters:**

   * ``config_json`` - Output pointer to allocated JSON string (caller frees)

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_NO_MEM`` - Allocation failure
   * ``ESP_ERR_INVALID_ARG`` - ``config_json`` is NULL

.. apifunction:: config_set_all_from_json
   :id: API_FUNC_CONFIG_SET_ALL_FROM_JSON
   :status: implemented
   :component: API_COMP_CONFIG_MANAGER
   :api_signature: esp_err_t config_set_all_from_json(const char *config_json)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_CFG_JSON_13
   
   Parse structured JSON array and update configuration atomically. Uses
   _no_commit setters internally and performs a single commit. Unknown keys
   are ignored.

   **Signature:**

   .. code-block:: c

      esp_err_t config_set_all_from_json(const char *config_json)

   **Parameters:**

   * ``config_json`` - JSON array string with key-type-value objects

   **Returns:**

   * ``ESP_OK`` - Success
   * ``ESP_ERR_INVALID_ARG`` - Validation failure or malformed JSON
   * ``ESP_ERR_NO_MEM`` - Memory allocation failure
   * ``ESP_ERR_NVS_*`` - NVS operation failure
