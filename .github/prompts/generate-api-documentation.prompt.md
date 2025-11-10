# API Documentation Generation Prompt

## Quick Usage

```text
Generate API documentation for: [HEADER_FILE_PATH]
Component ID: API_COMP_[NAME]
Output: docs/21_api/api_[component_name].rst
```

## Instructions

1. **Read the header file** and extract:
   - File-level documentation (@file, @brief, architecture, usage notes)
   - All public functions with their documentation
   - All structs, enums, typedefs (if present)
   - Requirement traceability (REQ_* mentions)

2. **Generate RST file** following this structure:

**CRITICAL CONSISTENCY RULES:**

- ✅ **EVERY** function MUST have the complete structure shown below
- ✅ **NO** abbreviated or minimal documentation - all functions get equal detail
- ✅ **ALL** sections are mandatory: Signature, Parameters, Returns
- ✅ Use "None" for Parameters if function takes no arguments
- ✅ Document ALL possible return values, not just success cases
- ❌ **NEVER** skip Signature/Parameters/Returns sections

```rst
Component Name API
==================

.. apicomponent:: Component Name
   :id: API_COMP_XXX
   :status: implemented
   :header_file: path/to/header.h
   :links: REQ_XXX, REQ_YYY
   
   **Brief Description**

   Component overview from file-level docstring.
   
   Key features:
   
   * Feature 1
   * Feature 2
   
   **Architecture:**
   
   Architecture notes from header.

Section Name (e.g., "Lifecycle Functions")
-------------------------------------------

.. apifunction:: function_name
   :id: API_FUNC_XXX
   :status: implemented
   :component: API_COMP_XXX
   :api_signature: return_type function_name(params)
   :returns: Summary for metadata
   :parameters: Summary for metadata
   :links: REQ_XXX
   
   Brief function description.

   **Signature:**

   .. code-block:: c

      return_type function_name(param_type param1,
                                param_type param2)

   **Parameters:**

   * ``param1`` - Parameter 1 description
   * ``param2`` - Parameter 2 description
   * None (if no parameters)

   **Returns:**

   * ``ESP_OK`` - Success case description
   * ``ESP_ERR_INVALID_ARG`` - Error case description
   * ``ESP_ERR_*`` - Other error cases

   .. note::
      Special notes: thread-safety, timing constraints, requirements, etc.

**REMEMBER:** Every single function must follow this exact structure - no exceptions!

Data Types Section (if structs/enums exist)
--------------------------------------------

.. apistruct:: struct_name
   :id: API_STRUCT_XXX
   :status: implemented
   :component: API_COMP_XXX
   :links: REQ_XXX
   
   Struct/enum description.

   **Definition:**

   .. code-block:: c

      typedef struct {
          type field1;
          type field2;
      } struct_name_t;

   **Fields:**

   * ``field1`` - Field description
   * ``field2`` - Field description

   OR for enums:

   **Values:**

   * ``ENUM_VALUE_1`` - Value description
   * ``ENUM_VALUE_2`` - Value description
```

### Step 3: Organize Functions

Group functions by their logical sections **as they appear in the header file**.
Common section patterns:

- Lifecycle Functions (init, start, stop, deinit)
- Configuration/Settings Functions
- Data Access/Query Functions  
- Utility/Helper Functions

Use the header file's existing organization and comments as the primary guide.

### Step 4: Extract Traceability

Extract requirement links from:

- File-level comments mentioning REQ_* IDs
- Function-level @requirement tags
- Inline requirement comments

### Step 5: Finalize

After generation:

- Add the new file to `docs/21_api/index.rst` toctree
- Verify all REQ_* links exist in `docs/11_requirements/`
- **Do NOT build docs automatically** - let user review first

## ID Naming Conventions

**Component IDs:** `API_COMP_[SHORTNAME]`

Examples:

- Config Manager → `API_COMP_CONFIG_MGR`
- Web Server → `API_COMP_WEB_SERVER`
- WiFi Manager → `API_COMP_WIFI_MGR`
- Certificate Handler → `API_COMP_CERT_HANDLER`
- Network Tunnel → `API_COMP_NETIF_TUNNEL`

**Function IDs:** `API_FUNC_[COMPONENT_SHORT]_[FUNCTION_NAME]`

Examples:

- `config_init()` → `API_FUNC_CONFIG_INIT`
- `wifi_manager_scan()` → `API_FUNC_WIFI_SCAN`
- `web_server_start()` → `API_FUNC_WEB_START`

**Struct IDs:** `API_STRUCT_[COMPONENT_SHORT]_[STRUCT_NAME]`

Examples:

- `wifi_status_t` → `API_STRUCT_WIFI_STATUS`
- `web_server_config_t` → `API_STRUCT_WEB_CONFIG`

**Important:** Use descriptive short names that match the component's purpose, not arbitrary abbreviations.

## Critical Notes

1. **Follow the RST structure template above** - This is the authoritative format
2. **Header file organization is primary** - Use the sections/grouping from the header
3. **All IDs must be unique** - Check existing API docs to avoid conflicts
4. **Requirement links must exist** - Verify REQ_* IDs in `docs/11_requirements/`
5. **No automatic build** - User reviews before building

## Quality Checklist

Before finalizing, verify:

- [ ] Every `.. apifunction::` has **Signature:**, **Parameters:**, **Returns:** sections
- [ ] No function is abbreviated or has missing sections
- [ ] All code blocks use `.. code-block:: c` with proper indentation
- [ ] All parameter names use inline code formatting (```param_name``)
- [ ] All return values are documented (ESP_OK, ESP_ERR_*, etc.)
- [ ] Structs/enums have **Definition:** and **Fields:**/**Values:** sections
- [ ] File added to `docs/21_api/index.rst` toctree
- [ ] All REQ_* links verified in requirements docs
