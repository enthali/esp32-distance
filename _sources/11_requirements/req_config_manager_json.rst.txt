JSON-Based Configuration Manager Requirements
===============================================

This document specifies component-level requirements for the JSON-Schema-Driven Configuration Management System. These requirements refine the high-level system requirement :need:`REQ_SYS_CFG_1` (Non-volatile Configuration Storage).

**Document Version**: 2.0  
**Last Updated**: 2025-11-12

Component Overview
------------------

The JSON-based configuration manager component provides centralized configuration management through a single JSON schema source of truth. Configuration is stored persistently in NVS and accessed via type-safe C API. The web interface generates forms dynamically from the schema.


Configuration Schema Definition
-------------------------------

.. req:: JSON Schema as Configuration Source of Truth
   :id: REQ_CFG_JSON_1
   :links: REQ_SYS_CFG_1
   :status: draft
   :priority: mandatory
   :tags: config, schema, architecture

   **Description:**
   The configuration manager SHALL define all configuration parameters in a single JSON schema file (config_schema.json) that serves as the authoritative source of truth for parameter definitions, default values, and UI metadata.

   **Rationale:**
   Single source of truth prevents duplication across C code, web forms, and documentation. Schema-driven approach enables code generation for factory defaults and dynamic UI generation.

   **Acceptance Criteria:**

   - AC-1: JSON schema file SHALL define all user-configurable parameters
   - AC-2: Schema SHALL include parameter type, label, default value, and validation constraints
   - AC-3: Parameters organized in groups for UI section structure
   - AC-4: Schema SHALL be embeddable in flash for browser access
   - AC-5: Schema format SHALL be human-readable and maintainable


.. req:: Parameter Grouping for UI Organization
   :id: REQ_CFG_JSON_2
   :links: REQ_CFG_JSON_1
   :status: draft
   :priority: optional
   :tags: config, ui, schema

   **Description:**
   The configuration schema SHALL support logical grouping of parameters into sections for organization in the web interface.

   **Rationale:**
   Logical organization improves usability for configuration with many parameters.

   **Acceptance Criteria:**

   - AC-1: Each group SHALL have id, label, and description
   - AC-2: Fields SHALL reference group by id
   - AC-3: UI SHALL display fields grouped and ordered as specified in schema
   - AC-4: Each group SHALL be collapsible/expandable (optional UI feature)


.. req:: Parameter Type System
   :id: REQ_CFG_JSON_3
   :links: REQ_CFG_JSON_1
   :status: draft
   :priority: mandatory
   :tags: config, types

   **Description:**
   The configuration system SHALL support basic parameter types: string, password, integer, boolean, and hidden.

   **Rationale:**
   Covers typical IoT configuration needs while remaining simple. Separate types enable appropriate UI widgets and validation.

   **Acceptance Criteria:**

   - AC-1: String type SHALL support min/max length and regex pattern validation
   - AC-2: Password type SHALL be string type with hidden display in UI
   - AC-3: Integer type SHALL support min/max range and step constraints
   - AC-4: Boolean type SHALL map to checkbox in UI
   - AC-5: Hidden type SHALL not appear in web form (internal config)


Configuration Generation and Defaults
-------------------------------------

.. req:: Build-Time Factory Defaults Generation
   :id: REQ_CFG_JSON_4
   :links: REQ_CFG_JSON_1, REQ_SYS_CFG_1
   :status: draft
   :priority: mandatory
   :tags: config, build, code-generation

   **Description:**
   The build system SHALL automatically generate C code to initialize factory default configuration from the JSON schema during build time.

   **Rationale:**
   Eliminates manual default value maintenance in C code. Generated code is compiled into firmware, ensuring defaults match schema without runtime JSON parsing.

   **Acceptance Criteria:**

   - AC-1: Python script SHALL parse config_schema.json during build
   - AC-2: Script SHALL generate config_factory_generated.c with config_write_factory_defaults() function
   - AC-3: Generated function SHALL call config_set_xxx() for each schema field with default value
   - AC-4: Build SHALL fail if schema is invalid JSON
   - AC-5: Generated code SHALL be included in firmware compilation


.. req:: No Runtime JSON Parsing in C Code
   :id: REQ_CFG_JSON_5
   :links: REQ_CFG_JSON_4
   :status: draft
   :priority: mandatory
   :tags: config, performance, memory

   **Description:**
   The ESP32 application code SHALL NOT parse JSON at runtime. JSON parsing SHALL occur only at build time (code generation) or in browser (UI).

   **Rationale:**
   Eliminates runtime overhead and memory overhead of JSON parser on resource-constrained ESP32. Simplifies C code and improves performance.

   **Acceptance Criteria:**

   - AC-1: C code SHALL NOT include JSON parser library
   - AC-2: NVS access SHALL use direct key-value API (no JSON deserialization)
   - AC-3: Factory reset SHALL call pre-generated config_write_factory_defaults() function
   - AC-4: Runtime memory usage SHALL not include JSON parser structures


NVS Storage and Access
----------------------

.. req:: Key-Based NVS Storage
   :id: REQ_CFG_JSON_6
   :links: REQ_CFG_JSON_1, REQ_SYS_CFG_1
   :status: draft
   :priority: mandatory
   :tags: config, storage, nvs

   **Description:**
   Configuration parameters SHALL be stored in NVS using the schema "key" field directly as the NVS key.

   **Rationale:**
   Direct key usage simplifies storage format and eliminates need for separate UUID system. Keys are human-readable for debugging.

   **Acceptance Criteria:**

   - AC-1: Each schema field key SHALL be valid NVS key (≤15 characters)
   - AC-2: NVS storage SHALL use "config" namespace for isolation
   - AC-3: Parameter values SHALL be stored individually (not as single blob)
   - AC-4: NVS key SHALL directly match schema field key


.. req:: Type-Safe Configuration API
   :id: REQ_CFG_JSON_7
   :links: REQ_CFG_JSON_6
   :status: draft
   :priority: mandatory
   :tags: config, api, type-safety

   **Description:**
   The configuration manager SHALL provide type-specific getter and setter functions for configuration access.

   **Rationale:**
   Type-specific functions enforce compile-time type checking via function signature. Different functions for different types prevent accidental type mismatches at call site.

   **Acceptance Criteria:**

   - AC-1: Getters SHALL be: config_get_string(), config_get_int32(), config_get_int16(), config_get_bool()
   - AC-2: Setters SHALL be: config_set_string(), config_set_int32(), config_set_int16(), config_set_bool()
   - AC-3: All functions SHALL take key as string parameter (matches schema key)
   - AC-4: Functions SHALL return ESP_OK or error code (ESP_ERR_INVALID_ARG, ESP_ERR_NVS_*)
   - AC-5: Caller SHALL use matching function for parameter type in schema


.. req:: Persistent Configuration Storage
   :id: REQ_CFG_JSON_8
   :links: REQ_CFG_JSON_6
   :status: draft
   :priority: mandatory
   :tags: config, storage, nvs

   **Description:**
   Configuration changes SHALL be immediately persisted to NVS on each config_set_xxx() call.

   **Rationale:**
   Ensures configuration survives power loss and system resets.

   **Acceptance Criteria:**

   - AC-1: config_set_xxx() SHALL write to NVS before returning
   - AC-2: Write failures SHALL be logged but not crash system
   - AC-3: Configuration SHALL be reloaded from NVS on next system boot
   - AC-4: NVS corruption SHALL be handled gracefully with defaults


.. req:: Factory Reset Capability
   :id: REQ_CFG_JSON_9
   :links: REQ_CFG_JSON_4, REQ_SYS_CFG_1
   :status: draft
   :priority: mandatory
   :tags: config, reset

   **Description:**
   The system SHALL provide config_factory_reset() function to reset all configuration to schema-defined defaults.

   **Rationale:**
   Users need ability to recover from misconfiguration without firmware rebuild.

   **Acceptance Criteria:**

   - AC-1: Factory reset SHALL erase all values in "config" NVS namespace
   - AC-2: Factory reset SHALL call config_write_factory_defaults() to restore defaults
   - AC-3: Factory reset SHALL return success/error status
   - AC-4: Factory reset errors SHALL be logged


Web Interface Integration
-------------------------

.. req:: Web Interface Integration Support
   :id: REQ_CFG_JSON_10
   :links: REQ_SYS_WEB_1, REQ_CFG_JSON_1, REQ_CFG_JSON_7
   :status: approved
   :priority: optional
   :tags: config, integration

   **Description:**
   The configuration system SHOULD provide integration points for web-based configuration interfaces.

   **Rationale:**
   Web servers need access to configuration schema and runtime values to provide user interfaces. Config Manager provides the data layer; Web Server provides the HTTP/UI layer.

   **Acceptance Criteria:**

   - AC-1: Configuration schema file (config_schema.json) SHALL be embeddable in web server
   - AC-2: C API functions (config_get_*, config_set_*) SHALL be callable from web request handlers
   - AC-3: Schema format SHALL support web form generation use cases
   - AC-4: API SHALL return error codes suitable for HTTP status mapping

   **Note:** 
   Detailed web interface requirements (HTML forms, REST API endpoints, validation) are specified in Web Server Requirements (:need:`REQ_WEB_CONF_1`). This requirement ensures Config Manager provides necessary integration points without knowing HTTP details.


Error Handling
--------------

.. req:: NVS Error Graceful Handling
   :id: REQ_CFG_JSON_11
   :links: REQ_CFG_JSON_6, REQ_SYS_REL_1
   :status: draft
   :priority: mandatory
   :tags: config, error-handling, reliability

   **Description:**
   The configuration system SHALL gracefully handle NVS errors and continue operating with default values.

   **Rationale:**
   NVS corruption or wear should not prevent system operation.

   **Acceptance Criteria:**

   - AC-1: NVS read failures SHALL log error and use schema default value
   - AC-2: NVS write failures SHALL log error but not crash application
   - AC-3: System SHALL boot successfully even if NVS namespace is corrupted
   - AC-4: Web interface SHALL indicate NVS error status to user


.. req:: Configuration Initialization on Boot
   :id: REQ_CFG_JSON_12
   :links: REQ_CFG_JSON_6, REQ_CFG_JSON_11
   :status: draft
   :priority: mandatory
   :tags: config, boot

   **Description:**
   On first boot, config_init() SHALL initialize NVS with factory defaults from schema if NVS "config" namespace is empty or uninitialized.

   **Rationale:**
   New devices need sensible defaults on first boot.

   **Acceptance Criteria:**

   - AC-1: config_init() SHALL check if NVS "config" namespace exists
   - AC-2: If empty, config_init() SHALL call config_factory_reset() to write defaults
   - AC-3: On subsequent boots, config_init() SHALL load values from NVS
   - AC-4: Process SHALL be atomic (no partial updates)


Development Guide
-----------------

.. req:: Simple Process to Add Configuration Fields
   :id: REQ_CFG_JSON_13
   :links: REQ_CFG_JSON_1
   :status: draft
   :priority: optional
   :tags: config, extensibility, developer-experience

   **Description:**
   Adding new configuration parameters SHALL require modification of only config_schema.json, with C code and web UI auto-updating.

   **Rationale:**
   Low friction for extending configuration reduces development time.

   **Acceptance Criteria:**

   - AC-1: New field added to config_schema.json SHALL appear in web form on next page reload
   - AC-2: Developer SHALL call config_get_xxx("key") in C code matching schema key
   - AC-3: Factory defaults auto-generated from schema (no manual C code needed)
   - AC-4: Documentation SHALL provide step-by-step guide for adding fields


.. req:: Type Safety via Optional Static Validation
   :id: REQ_CFG_JSON_14
   :links: REQ_CFG_JSON_7
   :status: draft
   :priority: optional
   :tags: config, validation, developer-experience

   **Description:**
   The build system SHOULD provide optional pre-build validation script that detects type mismatches between schema and C code.

   **Rationale:**
   Catches developer errors early without requiring code generation. Optional to keep build simple.

   **Acceptance Criteria:**

   - AC-1: Validator script SHALL parse config_schema.json
   - AC-2: Script SHALL search C source files for config_get_xxx("key") calls
   - AC-3: Script SHALL verify function type matches schema type
   - AC-4: Script SHALL output errors for mismatches
   - AC-5: Integration into build pipeline SHALL be optional (can be skipped)


Future Enhancements
-------------------

.. req:: Configuration Schema Versioning and Migration
   :id: REQ_CFG_JSON_15
   :links: REQ_CFG_JSON_1
   :status: open
   :priority: optional
   :tags: config, versioning, migration, future

   **Description:**
   The configuration system MAY support schema versioning and automatic migration of configuration data across firmware updates.

   **Rationale:**
   As firmware evolves, configuration structure may change (new fields, renamed keys, changed types). Schema versioning enables detection and migration of old configurations. However, embedded devices are typically "programmed out" thoroughly before production deployment, minimizing schema changes after deployment.

   **Acceptance Criteria (if implemented):**

   - AC-1: config_schema.json includes "schema_version" field
   - AC-2: config_init() detects NVS schema version mismatch
   - AC-3: Migration functions upgrade old config to new schema
   - AC-4: Migration preserves user settings where possible
   - AC-5: Failed migration triggers factory reset with user warning

   **Note:** This requirement is marked "open" as schema changes are rare in production embedded systems. Hardware constraints typically stabilize configuration structure early in development. For most deployments, factory reset on major firmware updates is acceptable alternative to complex migration logic.


Traceability
------------

All traceability is automatically generated by Sphinx-Needs based on the `:links:` attributes in each requirement.

.. needtable::
   :columns: id, title, status, tags

.. needflow:: REQ_CFG_JSON_1
