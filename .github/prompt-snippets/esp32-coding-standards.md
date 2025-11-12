# ESP32 Coding Standards

## General C Coding Guidelines

### Naming Conventions

- **Functions**: `snake_case` (e.g., `config_manager_init()`, `web_server_start()`)
- **Variables**: `snake_case` (e.g., `connection_status`, `buffer_size`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER_SIZE`, `DEFAULT_TIMEOUT`)
- **Macros**: `UPPER_SNAKE_CASE` (e.g., `#define TASK_STACK_SIZE 4096`)
- **Structs/Enums**: `snake_case_t` (e.g., `wifi_config_t`, `app_state_t`)

### ESP-IDF Specific Conventions

- **Component prefixes**: Use component name as prefix (e.g., `config_manager_save()`)
- **ESP-IDF functions**: Follow ESP-IDF naming (e.g., `esp_wifi_init()`, `esp_http_server_start()`)
- **Error handling**: Always use `ESP_ERROR_CHECK()` for critical operations
- **Logging**: Use ESP_LOG macros with appropriate log levels

### Memory Management

```c
// Prefer stack allocation for small, temporary data
uint8_t buffer[64];

// Use ESP-IDF memory functions for dynamic allocation
char* data = (char*)malloc(size);
if (data == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory");
    return ESP_ERR_NO_MEM;
}

// Always check return values
esp_err_t ret = esp_wifi_init(&wifi_init_config);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "WiFi init failed: %s", esp_err_to_name(ret));
    return ret;
}
```

### Error Handling Patterns

```c
// Standard ESP-IDF error handling
esp_err_t my_function(void) {
    esp_err_t ret = ESP_OK;
    
    ret = some_esp_function();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Operation failed: %s", esp_err_to_name(ret));
        goto cleanup;
    }
    
    // More operations...
    
cleanup:
    // Cleanup resources
    return ret;
}
```

### Logging Standards

```c
// Define TAG at top of file
static const char* TAG = "MY_COMPONENT";

// Use appropriate log levels
ESP_LOGI(TAG, "Component initialized successfully");
ESP_LOGW(TAG, "Operation timeout, retrying");
ESP_LOGE(TAG, "Critical failure in operation");
ESP_LOGD(TAG, "Debug: state=%d, retry=%d", state, retry_count);
```

### GPIO and Hardware Interface

```c
// Document pin assignments clearly
#define BUTTON_PIN     GPIO_NUM_0
#define STATUS_LED_PIN GPIO_NUM_2

// Use proper GPIO configuration
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << BUTTON_PIN),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE,
};
ESP_ERROR_CHECK(gpio_config(&io_conf));
```

### FreeRTOS Task Guidelines

```c
// Use proper task priorities (0-25, higher = more priority)
#define APP_TASK_PRIORITY       5
#define NETWORK_TASK_PRIORITY   6

// Use reasonable stack sizes
#define APP_TASK_STACK_SIZE     2048
#define NETWORK_TASK_STACK_SIZE 4096

// Proper task creation
xTaskCreate(app_task, "app_task", APP_TASK_STACK_SIZE, 
            NULL, APP_TASK_PRIORITY, NULL);
```

**Component Structure:** See build-instructions.md for detailed component folder structure and troubleshooting guide.

### Header File Guards

```c
#ifndef MY_COMPONENT_H
#define MY_COMPONENT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declarations here

#ifdef __cplusplus
}
#endif

#endif // MY_COMPONENT_H
```

### Function Documentation

```c
/**
 * @brief Initialize configuration manager
 * 
 * Loads configuration from NVS storage and initializes runtime structures.
 * Creates default configuration if no saved configuration exists.
 * 
 * @return ESP_OK on success, ESP_ERR_NO_MEM on allocation failure,
 *         ESP_ERR_NVS_* on storage errors
 */
esp_err_t config_manager_init(void);
```

### File Documentation

```c
/**
 * @file config_manager.c
 * @brief Configuration management implementation
 * 
 * Provides functions to load, save, and manage application configuration
 * using ESP32 NVS (Non-Volatile Storage).
 * 
 * @author ESP32 Template Project
 * @date 2025
 */
```


**Documentation Requirements:**

- Every function must have a Doxygen comment explaining purpose, parameters, and return values
- Every file must have a header comment describing the module's purpose
- Complex algorithms must include inline comments explaining the logic
- Hardware dependencies must be documented (pin assignments, timing constraints)

### Configuration Management

```c
// Use Kconfig for configurable parameters
#ifdef CONFIG_APP_MAX_RETRIES
#define MAX_RETRIES CONFIG_APP_MAX_RETRIES
#else
#define MAX_RETRIES 3  // Default value
#endif
```

## ESP32-Specific Best Practices

### IRAM Usage

```c
// Only use IRAM_ATTR for time-critical interrupt handlers
IRAM_ATTR void gpio_isr_handler(void* arg) {
    // Keep ISR code minimal and fast
}
```

### WiFi Event Handling

```c
// Proper event handler registration
ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                          &wifi_event_handler, NULL));
ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                          &ip_event_handler, NULL));
```

### Memory Efficiency

- Use `const` for read-only data to place it in flash
- Use appropriate data types (`uint8_t` vs `uint32_t`)
- Consider using packed structs for wire protocols
- Free allocated memory promptly

### Component Dependencies

```cmake
# In CMakeLists.txt
idf_component_register(
    SRCS "src/my_component.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_wifi esp_http_server
    PRIV_REQUIRES esp_timer
)
```

## Documentation Standards

### Sphinx-Needs Documentation Methodology

This project uses **Sphinx-Needs** for professional requirements and design documentation:

#### Requirements Documentation

Requirements are documented in reStructuredText (`.rst`) files in `docs/11_requirements/`:

```rst
.. req:: Configuration Manager Initialization
   :id: REQ_CFG_1
   :links: REQ_SYS_CFG_1
   :status: approved
   :priority: mandatory
   :tags: config, initialization

   **Description:** The system SHALL initialize configuration manager on boot.

   **Acceptance Criteria:**
   - AC-1: Configuration loaded from NVS within 500ms
   - AC-2: Default values used if NVS is empty
```

#### Design Specifications

Design documents are maintained separately in `docs/12_design/` to avoid code bloat:

- Detailed architectural decisions
- Component interaction diagrams  
- Algorithm specifications
- State machine definitions

Design specs link to requirements using `:links:` attribute for traceability.

#### Code-Level Traceability

Code should reference requirement and design IDs in comments:

```c
/**
 * @file config_manager.c
 * @brief Configuration management implementation
 * 
 * REQUIREMENTS TRACEABILITY:
 * - REQ_CFG_1: Configuration initialization
 * - REQ_CFG_3: Runtime configuration structure
 * - REQ_CFG_4: NVS persistent storage
 */

/**
 * @brief Initialize configuration manager - implements REQ_CFG_1
 * 
 * Loads configuration from NVS or uses defaults if not found.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t config_manager_init(void) {
    // Implementation
}
```

#### Benefits of Sphinx-Needs

- ✅ **Separate concerns**: Design docs don't bloat code files
- ✅ **Professional tooling**: Auto-generated traceability matrices and graphs
- ✅ **Version controlled**: Design docs tracked in Git like code
- ✅ **Bidirectional links**: Requirements ↔ Design ↔ Code
- ✅ **Scalable**: Works for projects of any size

## See Also

- [Build Instructions](build-instructions.md) - Component structure and troubleshooting
- [Development Workflow](development.md) - Branch strategy and PR process
- [Commit Message Guidelines](commit-message.md) - Commit format and scopes
