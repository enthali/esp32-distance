Configuration Webpage Design Specification
===========================================

This document specifies the design of the browser-based configuration interface that provides schema-driven dynamic form generation, bulk configuration management, and factory reset capabilities.


Architecture Overview
---------------------

.. spec:: Configuration Webpage Architecture
   :id: SPEC_CFG_WEB_ARCH_1
   :links: REQ_WEB_CONF_1, REQ_CFG_JSON_12, REQ_CFG_JSON_13
   :status: open
   :tags: frontend, javascript, ui

   **Overview:**
   The configuration webpage is a single-page JavaScript application that dynamically generates configuration forms from JSON schema and manages all configuration operations through RESTful bulk APIs.

   **Architecture Layers:**

   1. **Schema Processing Layer**:
      - Fetch JSON schema from ``GET /api/config/schema``
      - Parse schema field definitions (key, type, label, description, validation)
      - Cache schema for form generation and validation

   2. **Form Generation Layer**:
      - Dynamic HTML form element creation based on schema types
      - Input type mapping (string → text, integer → number, boolean → checkbox)
      - Validation attribute generation (min, max, pattern, required)
      - Label and description rendering

   3. **Data Management Layer**:
      - Fetch configuration as structured JSON array ``GET /api/config``
      - Parse ``{key, type, value}`` objects
      - Update form fields with current values
      - Collect form data and generate structured JSON for save operations

   4. **API Integration Layer**:
      - HTTP client for REST API communication
      - Error handling and retry logic
      - Response parsing and status code handling
      - Loading state management

   5. **User Interaction Layer**:
      - Save button: Collect form → Generate JSON → ``POST /api/config``
      - Restore button: Fetch current config → Update form
      - Factory Reset button: Confirmation dialog → ``POST /api/config/reset``
      - Reset countdown modal with timer display

   **Component Dependencies:**
   
   - ``/api/config/schema``: JSON schema endpoint (config_manager)
   - ``/api/config``: Bulk configuration GET/POST endpoint (web_server → config_manager)
   - ``/api/config/reset``: Factory reset endpoint (web_server → config_manager)


Data Flow Architecture
----------------------

.. spec:: Configuration Data Flow
   :id: SPEC_CFG_WEB_FLOW_1
   :links: SPEC_CFG_WEB_ARCH_1, SPEC_CFG_JSON_BULK_1
   :status: open
   :tags: data-flow, json, api

   **Page Load Sequence:**

   ::

      1. Browser loads settings.html
         ↓
      2. JavaScript fetches schema: GET /api/config/schema
         ↓
      3. Parse schema → Generate form HTML dynamically
         ↓
      4. Fetch current config: GET /api/config
         ↓
      5. Parse [{key, type, value}, ...] → Update form fields
         ↓
      6. Enable user interaction (Save, Restore, Factory Reset)

   **Save Configuration Flow:**

   ::

      1. User clicks "Save" button
         ↓
      2. JavaScript collects form values
         ↓
      3. Build structured JSON array:
         [
           {key: "wifi_ssid", type: "string", value: "MyNetwork"},
           {key: "led_count", type: "integer", value: 50},
           ...
         ]
         ↓
      4. POST /api/config with JSON body
         ↓
      5. Server responds: 200 OK or error
         ↓
      6. Show success message + reset countdown modal
         ↓
      7. Auto-reload page after countdown (device reboots)

   **Restore Configuration Flow:**

   ::

      1. User clicks "Restore" button
         ↓
      2. Fetch current config: GET /api/config
         ↓
      3. Parse structured JSON array
         ↓
      4. Update all form fields with current values
         ↓
      5. Show success message

   **Factory Reset Flow:**

   ::

      1. User clicks "Factory Reset" button
         ↓
      2. Show confirmation dialog: "This will reset all settings!"
         ↓
      3. User confirms
         ↓
      4. POST /api/config/reset
         ↓
      5. Server responds: 200 OK (device will reboot)
         ↓
      6. Show countdown modal: "Device resetting in 5...4...3...2...1"
         ↓
      7. Auto-redirect to WiFi setup page (device rebooted to defaults)


Schema-Driven Form Generation
------------------------------

.. spec:: Dynamic Form Generation from Schema
   :id: SPEC_CFG_WEB_FORM_1
   :links: SPEC_CFG_WEB_ARCH_1, SPEC_CFG_JSON_SCHEMA_1
   :status: open
   :tags: form-generation, schema, javascript

   **Description:** JavaScript code that dynamically creates HTML form elements based on JSON schema field definitions.

   **Schema Field Processing:**

   .. code-block:: javascript

      // Input: JSON schema from GET /api/config/schema
      const schema = {
        "fields": [
          {
            "key": "wifi_ssid",
            "type": "string",
            "label": "WiFi SSID",
            "description": "Network name to connect",
            "default": "",
            "validation": {
              "required": true,
              "maxLength": 32
            }
          },
          {
            "key": "led_count",
            "type": "integer",
            "label": "LED Count",
            "description": "Number of LEDs in strip",
            "default": 50,
            "validation": {
              "min": 1,
              "max": 300
            }
          }
        ]
      };

      // Process each field to generate form HTML
      function generateForm(schema) {
        const formContainer = document.getElementById('config-form');
        
        schema.fields.forEach(field => {
          const formGroup = createFormGroup(field);
          formContainer.appendChild(formGroup);
        });
      }

   **Form Element Generation Logic:**

   .. code-block:: javascript

      function createFormGroup(field) {
        const group = document.createElement('div');
        group.className = 'form-group';
        
        // Create label
        const label = document.createElement('label');
        label.textContent = field.label;
        label.htmlFor = field.key;
        
        // Create input based on type
        let input;
        switch(field.type) {
          case 'string':
            input = document.createElement('input');
            input.type = field.key.includes('pass') ? 'password' : 'text';
            if (field.validation?.maxLength) {
              input.maxLength = field.validation.maxLength;
            }
            if (field.validation?.pattern) {
              input.pattern = field.validation.pattern;
            }
            break;
            
          case 'integer':
            input = document.createElement('input');
            input.type = 'number';
            if (field.validation?.min !== undefined) {
              input.min = field.validation.min;
            }
            if (field.validation?.max !== undefined) {
              input.max = field.validation.max;
            }
            break;
            
          case 'boolean':
            input = document.createElement('input');
            input.type = 'checkbox';
            break;
        }
        
        input.id = field.key;
        input.name = field.key;
        input.required = field.validation?.required || false;
        
        // Create description
        const description = document.createElement('small');
        description.className = 'form-text';
        description.textContent = field.description;
        
        group.appendChild(label);
        group.appendChild(input);
        group.appendChild(description);
        
        return group;
      }

   **Design Properties:**

   - **No hardcoded form fields**: All HTML generated from schema
   - **Type-safe input elements**: Schema type maps to correct HTML input type
   - **Automatic validation**: HTML5 validation attributes from schema
   - **Password detection**: Fields with "pass" in key get type="password"
   - **Extensible**: New schema fields automatically generate form elements


Configuration Data Mapping
---------------------------

.. spec:: JSON Array to Form Field Mapping
   :id: SPEC_CFG_WEB_MAPPING_1
   :links: SPEC_CFG_WEB_ARCH_1, SPEC_CFG_JSON_BULK_1
   :status: open
   :tags: data-mapping, json, form

   **Description:** JavaScript logic for loading structured JSON configuration data into form fields and collecting form data back into structured JSON format.

   **Loading Configuration into Form:**

   .. code-block:: javascript

      // Input: Structured JSON array from GET /api/config
      async function loadConfiguration() {
        try {
          const response = await fetch('/api/config');
          const configArray = await response.json();
          
          // Example configArray:
          // [
          //   {key: "wifi_ssid", type: "string", value: "MyNetwork"},
          //   {key: "led_count", type: "integer", value: 50},
          //   {key: "enable_leds", type: "boolean", value: true}
          // ]
          
          configArray.forEach(item => {
            const input = document.getElementById(item.key);
            if (!input) return; // Field not in form (schema mismatch)
            
            switch(item.type) {
              case 'string':
              case 'integer':
                input.value = item.value;
                break;
              case 'boolean':
                input.checked = item.value;
                break;
            }
          });
          
          showMessage('Configuration loaded', 'success');
        } catch (error) {
          showMessage('Failed to load configuration: ' + error.message, 'error');
        }
      }

   **Collecting Form Data into Structured JSON:**

   .. code-block:: javascript

      function collectFormData(schema) {
        const configArray = [];
        
        schema.fields.forEach(field => {
          const input = document.getElementById(field.key);
          if (!input) return;
          
          let value;
          switch(field.type) {
            case 'string':
              value = input.value;
              break;
            case 'integer':
              value = parseInt(input.value, 10);
              break;
            case 'boolean':
              value = input.checked;
              break;
          }
          
          configArray.push({
            key: field.key,
            type: field.type,
            value: value
          });
        });
        
        return configArray;
      }

   **Save Configuration to Server:**

   .. code-block:: javascript

      async function saveConfiguration() {
        try {
          // Validate form first
          const form = document.getElementById('config-form');
          if (!form.checkValidity()) {
            form.reportValidity();
            return;
          }
          
          // Collect form data as structured JSON
          const configArray = collectFormData(cachedSchema);
          
          // POST to server
          const response = await fetch('/api/config', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify(configArray)
          });
          
          if (!response.ok) {
            throw new Error('Server returned ' + response.status);
          }
          
          showMessage('Configuration saved successfully', 'success');
          showResetCountdown();
          
        } catch (error) {
          showMessage('Failed to save configuration: ' + error.message, 'error');
        }
      }

   **Design Properties:**

   - **Type-aware mapping**: Converts between HTML input values and typed JSON
   - **Schema-driven**: Uses cached schema for type information
   - **Validation before save**: HTML5 form validation prevents invalid data
   - **Error handling**: Network errors and HTTP errors handled gracefully


User Interface States
----------------------

.. spec:: UI State Management
   :id: SPEC_CFG_WEB_STATE_1
   :links: SPEC_CFG_WEB_ARCH_1
   :status: open
   :tags: ui-state, javascript

   **Description:** JavaScript state management for different UI states during page lifecycle.

   **State Definitions:**

   ====================  ====================================  ===================================
   State                 UI Behavior                           User Actions Allowed
   ====================  ====================================  ===================================
   ``LOADING``           Loading spinner visible               None (buttons disabled)
   ``READY``             Form visible and enabled              Save, Restore, Factory Reset
   ``SAVING``            Save button shows spinner             None (all buttons disabled)
   ``ERROR``             Error message displayed               Retry allowed
   ``COUNTDOWN``         Modal with countdown timer            None (auto-reload pending)
   ====================  ====================================  ===================================

   **State Transition Logic:**

   .. code-block:: javascript

      let currentState = 'LOADING';
      
      function setState(newState) {
        currentState = newState;
        
        // Update UI based on state
        const form = document.getElementById('config-form');
        const saveBtn = document.getElementById('save-btn');
        const restoreBtn = document.getElementById('restore-btn');
        const resetBtn = document.getElementById('factory-reset-btn');
        const spinner = document.getElementById('loading-spinner');
        
        switch(newState) {
          case 'LOADING':
            form.style.display = 'none';
            spinner.style.display = 'block';
            saveBtn.disabled = true;
            restoreBtn.disabled = true;
            resetBtn.disabled = true;
            break;
            
          case 'READY':
            form.style.display = 'block';
            spinner.style.display = 'none';
            saveBtn.disabled = false;
            restoreBtn.disabled = false;
            resetBtn.disabled = false;
            break;
            
          case 'SAVING':
            saveBtn.disabled = true;
            saveBtn.innerHTML = '<span class="spinner"></span> Saving...';
            restoreBtn.disabled = true;
            resetBtn.disabled = true;
            break;
            
          case 'ERROR':
            form.style.display = 'block';
            spinner.style.display = 'none';
            saveBtn.disabled = false;
            restoreBtn.disabled = false;
            resetBtn.disabled = false;
            break;
            
          case 'COUNTDOWN':
            showModal('countdown-modal');
            saveBtn.disabled = true;
            restoreBtn.disabled = true;
            resetBtn.disabled = true;
            break;
        }
      }


Reset Countdown Modal
---------------------

.. spec:: Device Reset Countdown Interface
   :id: SPEC_CFG_WEB_COUNTDOWN_1
   :links: SPEC_CFG_WEB_ARCH_1
   :status: open
   :tags: ui, countdown, reset

   **Description:** Modal dialog with countdown timer shown after configuration save or factory reset to inform user of pending device reboot.

   **Modal HTML Structure:**

   .. code-block:: html

      <div id="countdown-modal" class="modal">
        <div class="modal-content">
          <h2>Configuration Saved</h2>
          <p>Device will restart in <span id="countdown">5</span> seconds...</p>
          <p class="warning">Page will reload automatically after restart.</p>
        </div>
      </div>

   **Countdown JavaScript Logic:**

   .. code-block:: javascript

      function showResetCountdown(seconds = 5) {
        setState('COUNTDOWN');
        
        const countdownEl = document.getElementById('countdown');
        let remaining = seconds;
        
        const interval = setInterval(() => {
          remaining--;
          countdownEl.textContent = remaining;
          
          if (remaining <= 0) {
            clearInterval(interval);
            // Device should have rebooted, attempt page reload
            window.location.reload();
          }
        }, 1000);
      }

   **Factory Reset Countdown Variant:**

   .. code-block:: javascript

      async function factoryReset() {
        // Show confirmation dialog
        const confirmed = confirm(
          'Factory Reset will restore all settings to defaults.\n\n' +
          'WiFi credentials will be erased.\n' +
          'Device will restart in AP mode.\n\n' +
          'Are you sure?'
        );
        
        if (!confirmed) return;
        
        try {
          const response = await fetch('/api/config/reset', {
            method: 'POST'
          });
          
          if (!response.ok) {
            throw new Error('Reset failed: ' + response.status);
          }
          
          // Show countdown modal
          showFactoryResetCountdown(5);
          
        } catch (error) {
          showMessage('Factory reset failed: ' + error.message, 'error');
        }
      }
      
      function showFactoryResetCountdown(seconds = 5) {
        setState('COUNTDOWN');
        
        // Update modal text for factory reset
        const modal = document.getElementById('countdown-modal');
        modal.querySelector('h2').textContent = 'Factory Reset Initiated';
        modal.querySelector('.warning').textContent = 
          'Device resetting to defaults. Redirecting to WiFi setup...';
        
        const countdownEl = document.getElementById('countdown');
        let remaining = seconds;
        
        const interval = setInterval(() => {
          remaining--;
          countdownEl.textContent = remaining;
          
          if (remaining <= 0) {
            clearInterval(interval);
            // Redirect to WiFi setup page (device now in AP mode)
            window.location.href = '/wifi-setup.html';
          }
        }, 1000);
      }

   **Design Properties:**

   - **Non-blocking UI**: Modal overlay prevents user interaction during countdown
   - **Clear messaging**: User knows exactly what's happening and when
   - **Automatic recovery**: Page reloads/redirects without user action
   - **Different variants**: Save countdown vs Factory Reset countdown with appropriate messaging


Error Handling
--------------

.. spec:: Error Handling and User Feedback
   :id: SPEC_CFG_WEB_ERROR_1
   :links: SPEC_CFG_WEB_ARCH_1
   :status: open
   :tags: error-handling, feedback

   **Description:** Comprehensive error handling for all API operations with user-friendly feedback.

   **Error Categories:**

   1. **Network Errors**: Fetch timeout, connection refused, DNS failure
   2. **HTTP Errors**: 4xx client errors, 5xx server errors
   3. **Validation Errors**: Invalid form data, schema mismatch
   4. **Parse Errors**: Malformed JSON responses

   **Error Handling Implementation:**

   .. code-block:: javascript

      async function apiCall(url, options = {}) {
        try {
          const response = await fetch(url, {
            ...options,
            timeout: 10000 // 10 second timeout
          });
          
          if (!response.ok) {
            // HTTP error
            let errorMsg = `HTTP ${response.status}`;
            
            // Try to parse error response
            try {
              const errorData = await response.json();
              if (errorData.error) {
                errorMsg = errorData.error;
              }
            } catch (e) {
              // Response not JSON, use default message
            }
            
            throw new Error(errorMsg);
          }
          
          // Parse successful response
          const data = await response.json();
          return data;
          
        } catch (error) {
          // Network error or other exception
          if (error.name === 'AbortError') {
            throw new Error('Request timeout - device may be offline');
          }
          throw error;
        }
      }

   **User Feedback Display:**

   .. code-block:: javascript

      function showMessage(message, type = 'info') {
        const messageEl = document.getElementById('message-banner');
        
        messageEl.textContent = message;
        messageEl.className = `message ${type}`; // 'success', 'error', 'warning', 'info'
        messageEl.style.display = 'block';
        
        // Auto-hide success messages after 5 seconds
        if (type === 'success') {
          setTimeout(() => {
            messageEl.style.display = 'none';
          }, 5000);
        }
      }

   **Retry Logic for Critical Operations:**

   .. code-block:: javascript

      async function loadSchemaWithRetry(maxRetries = 3) {
        let lastError;
        
        for (let i = 0; i < maxRetries; i++) {
          try {
            const schema = await apiCall('/api/config/schema');
            return schema;
          } catch (error) {
            lastError = error;
            if (i < maxRetries - 1) {
              // Wait before retry: 1s, 2s, 4s
              await sleep(1000 * Math.pow(2, i));
            }
          }
        }
        
        throw new Error(`Failed to load schema after ${maxRetries} attempts: ${lastError.message}`);
      }


Page Initialization Sequence
-----------------------------

.. spec:: Complete Page Initialization Flow
   :id: SPEC_CFG_WEB_INIT_1
   :links: SPEC_CFG_WEB_ARCH_1
   :status: open
   :tags: initialization, lifecycle

   **Description:** Complete initialization sequence when configuration page loads.

   **Initialization Code:**

   .. code-block:: javascript

      // Global cached schema
      let cachedSchema = null;
      
      // DOMContentLoaded event handler
      document.addEventListener('DOMContentLoaded', async function() {
        try {
          setState('LOADING');
          
          // Step 1: Load JSON schema with retry
          showMessage('Loading configuration schema...', 'info');
          cachedSchema = await loadSchemaWithRetry(3);
          
          // Step 2: Generate form from schema
          showMessage('Generating form...', 'info');
          generateForm(cachedSchema);
          
          // Step 3: Load current configuration
          showMessage('Loading current configuration...', 'info');
          await loadConfiguration();
          
          // Step 4: Attach event handlers
          document.getElementById('save-btn').addEventListener('click', saveConfiguration);
          document.getElementById('restore-btn').addEventListener('click', loadConfiguration);
          document.getElementById('factory-reset-btn').addEventListener('click', factoryReset);
          
          // Step 5: Ready for user interaction
          setState('READY');
          showMessage('Configuration loaded successfully', 'success');
          
        } catch (error) {
          setState('ERROR');
          showMessage('Failed to initialize page: ' + error.message, 'error');
          
          // Show retry button
          const retryBtn = document.getElementById('retry-btn');
          retryBtn.style.display = 'block';
          retryBtn.addEventListener('click', () => {
            window.location.reload();
          });
        }
      });

   **Initialization Sequence Diagram:**

   ::

      Page Load
        ↓
      DOMContentLoaded event
        ↓
      setState('LOADING')
        ↓
      Fetch /api/config/schema (with retry)
        ↓
      Parse schema → cachedSchema
        ↓
      generateForm(cachedSchema)
        ↓
      Fetch /api/config
        ↓
      Parse config array → Update form fields
        ↓
      Attach button event handlers
        ↓
      setState('READY')
        ↓
      User can interact


Traceability
------------

.. needtable::
   :columns: id, title, status
   :filter: id.startswith('SPEC_CFG_WEB_')

.. needflow:: SPEC_CFG_WEB_ARCH_1
