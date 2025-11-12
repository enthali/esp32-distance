/**
 * @file config_manager.c
 * @brief JSON Schema-Driven Configuration Manager Implementation
 * 
 * Simple NVS wrapper with no validation logic. Browser performs validation.
 * Direct key-value storage - no enums, no metadata tables, no runtime caches.
 * 
 * ARCHITECTURE:
 * - JSON schema (config_schema.json) defines all parameters
 * - Python script generates factory defaults at build time
 * - config_get/set functions are thin wrappers around NVS API
 * - Zero RAM overhead (direct NVS access, no caches)
 * 
 * @author ESP32 Distance Sensor Project
 * @date 2025
 * @version 3.0 (JSON-based)
 * 
 * Requirements Traceability:
 * - REQ_CFG_JSON_6: Key-Based NVS Storage
 * - REQ_CFG_JSON_7: Type-Safe Configuration API
 * - REQ_CFG_JSON_8: Persistent Configuration Storage
 * - REQ_CFG_JSON_11: NVS Error Graceful Handling
 * - REQ_CFG_JSON_12: Configuration Initialization on Boot
 */

#include "config_manager.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include <string.h>

// Embedded schema from EMBED_FILES in CMakeLists.txt
extern const char config_schema_json_start[] asm("_binary_config_schema_json_start");
extern const char config_schema_json_end[] asm("_binary_config_schema_json_end");
#include <string.h>

static const char *TAG = "config";

// =============================================================================
// PRIVATE VARIABLES
// =============================================================================

/** @brief NVS namespace for configuration storage */
#define NVS_NAMESPACE "config"

/** @brief NVS handle for config namespace */
static nvs_handle_t config_nvs_handle = 0;

/** @brief Initialization flag */
static bool config_initialized = false;

// =============================================================================
// PRIVATE HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Open NVS handle if not already open
 * @return ESP_OK on success, error code otherwise
 */
static esp_err_t ensure_nvs_open(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Config manager not initialized - call config_init() first");
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t config_commit(void) {
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_commit(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit NVS changes: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "NVS changes committed successfully");
    return ESP_OK;
}

// =============================================================================
// LIFECYCLE FUNCTIONS (REQ_CFG_JSON_12)
// =============================================================================

esp_err_t config_init(void) {
    if (config_initialized) {
        ESP_LOGW(TAG, "Config manager already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing configuration manager...");
    
    // Initialize NVS if not already done
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated, erase and re-initialize
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Open NVS namespace "config"
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace '%s': %s", 
                 NVS_NAMESPACE, esp_err_to_name(ret));
        return ret;
    }
    
    config_initialized = true;
    ESP_LOGI(TAG, "Configuration manager initialized successfully");
    
    // Validate configuration completeness (auto-generated check)
    // If any required keys are missing, factory defaults are written and system restarts
    config_validate_or_reset();
    
    return ESP_OK;
}

esp_err_t config_factory_reset(void) {
    ESP_LOGI(TAG, "Resetting configuration to factory defaults...");
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Erase all keys in "config" namespace
    ret = nvs_erase_all(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase NVS namespace: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Write factory defaults (auto-generated function)
    config_write_factory_defaults();
    
    // Commit changes
    ret = nvs_commit(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit factory defaults to NVS: %s", 
                 esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Factory defaults written successfully");
    return ESP_OK;
}

// =============================================================================
// STRING PARAMETER ACCESS (REQ_CFG_JSON_7)
// =============================================================================

esp_err_t config_get_string(const char* key, char* buffer, size_t buf_len) {
    if (key == NULL || buffer == NULL) {
        ESP_LOGE(TAG, "config_get_string: key or buffer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    size_t required_size = buf_len;
    ret = nvs_get_str(config_nvs_handle, key, buffer, &required_size);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        buffer[0] = '\0';  // Return empty string
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read string key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t config_set_string_no_commit(const char* key, const char* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_set_string_no_commit: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Write to NVS without commit
    ret = nvs_set_str(config_nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write string key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set string '%s' = '%s' (no commit)", key, value);
    return ESP_OK;
}

esp_err_t config_set_string(const char* key, const char* value) {
    esp_err_t ret = config_set_string_no_commit(key, value);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Commit immediately for persistence
    return config_commit();
}

// =============================================================================
// INTEGER PARAMETER ACCESS (REQ_CFG_JSON_7)
// =============================================================================

esp_err_t config_get_int32(const char* key, int32_t* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_get_int32: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_get_i32(config_nvs_handle, key, value);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        *value = 0;  // Return 0 as default
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read int32 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t config_set_int32(const char* key, int32_t value) {
    if (key == NULL) {
        ESP_LOGE(TAG, "config_set_int32: key is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Write to NVS
    ret = nvs_set_i32(config_nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write int32 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    // Commit immediately for persistence
    ret = nvs_commit(config_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit int32 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set int32 '%s' = %ld", key, (long)value);
    return ESP_OK;
}

esp_err_t config_get_int16(const char* key, int16_t* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_get_int16: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = nvs_get_i16(config_nvs_handle, key, value);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        *value = 0;  // Return 0 as default
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read int16 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t config_set_int16_no_commit(const char* key, int16_t value) {
    if (key == NULL) {
        ESP_LOGE(TAG, "config_set_int16_no_commit: key is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Write to NVS without commit
    ret = nvs_set_i16(config_nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write int16 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set int16 '%s' = %d (no commit)", key, value);
    return ESP_OK;
}

esp_err_t config_set_int16(const char* key, int16_t value) {
    esp_err_t ret = config_set_int16_no_commit(key, value);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Commit immediately for persistence
    return config_commit();
}

esp_err_t config_set_int32_no_commit(const char* key, int32_t value) {
    if (key == NULL) {
        ESP_LOGE(TAG, "config_set_int32_no_commit: key is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Write to NVS without commit
    ret = nvs_set_i32(config_nvs_handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write int32 key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set int32 '%s' = %" PRId32 " (no commit)", key, value);
    return ESP_OK;
}

// =============================================================================
// BOOLEAN PARAMETER ACCESS (REQ_CFG_JSON_7)
// =============================================================================

esp_err_t config_get_bool(const char* key, bool* value) {
    if (key == NULL || value == NULL) {
        ESP_LOGE(TAG, "config_get_bool: key or value is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    uint8_t u8_value;
    ret = nvs_get_u8(config_nvs_handle, key, &u8_value);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
        *value = false;  // Return false as default
        return ESP_ERR_NVS_NOT_FOUND;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read bool key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    *value = (u8_value != 0);
    return ESP_OK;
}

esp_err_t config_set_bool_no_commit(const char* key, bool value) {
    if (key == NULL) {
        ESP_LOGE(TAG, "config_set_bool_no_commit: key is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = ensure_nvs_open();
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Store bool as uint8
    uint8_t u8_value = value ? 1 : 0;
    
    // Write to NVS without commit
    ret = nvs_set_u8(config_nvs_handle, key, u8_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write bool key '%s': %s", 
                 key, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGD(TAG, "Set bool '%s' = %s (no commit)", key, value ? "true" : "false");
    return ESP_OK;
}

esp_err_t config_set_bool(const char* key, bool value) {
    esp_err_t ret = config_set_bool_no_commit(key, value);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Commit immediately for persistence
    return config_commit();
}

// =============================================================================
// BULK JSON CONFIGURATION API (REQ_CFG_JSON_12, REQ_CFG_JSON_13)
// =============================================================================

esp_err_t config_get_schema_json(char **schema_json) {
    if (schema_json == NULL) {
        ESP_LOGE(TAG, "config_get_schema_json: schema_json is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Calculate schema size
    size_t schema_size = config_schema_json_end - config_schema_json_start;
    
    if (schema_size == 0) {
        ESP_LOGE(TAG, "Embedded schema is empty or not found");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Return pointer to embedded schema (no allocation needed)
    *schema_json = (char*)config_schema_json_start;
    
    ESP_LOGD(TAG, "Returned embedded schema (%zu bytes)", schema_size);
    return ESP_OK;
}

esp_err_t config_get_all_as_json(char **config_json) {
    if (config_json == NULL) {
        ESP_LOGE(TAG, "config_get_all_as_json: config_json is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Parse embedded schema
    char *schema_str = NULL;
    esp_err_t ret = config_get_schema_json(&schema_str);
    if (ret != ESP_OK) {
        return ret;
    }
    
    cJSON *schema = cJSON_Parse(schema_str);
    if (schema == NULL) {
        ESP_LOGE(TAG, "Failed to parse schema JSON");
        return ESP_ERR_INVALID_ARG;
    }
    
    cJSON *fields = cJSON_GetObjectItem(schema, "parameters");
    if (!cJSON_IsArray(fields)) {
        ESP_LOGE(TAG, "Schema missing 'parameters' array");
        cJSON_Delete(schema);
        return ESP_ERR_INVALID_ARG;
    }
    
    // Build config array
    cJSON *config_array = cJSON_CreateArray();
    if (config_array == NULL) {
        ESP_LOGE(TAG, "Failed to create JSON array");
        cJSON_Delete(schema);
        return ESP_ERR_NO_MEM;
    }
    
    // Iterate through schema fields
    cJSON *field = NULL;
    cJSON_ArrayForEach(field, fields) {
        cJSON *key_item = cJSON_GetObjectItem(field, "key");
        cJSON *type_item = cJSON_GetObjectItem(field, "type");
        
        if (!cJSON_IsString(key_item) || !cJSON_IsString(type_item)) {
            continue; // Skip invalid field
        }
        
        const char *key = key_item->valuestring;
        const char *type = type_item->valuestring;
        
        // Create config entry object
        cJSON *entry = cJSON_CreateObject();
        if (entry == NULL) {
            continue;
        }
        
        cJSON_AddStringToObject(entry, "key", key);
        cJSON_AddStringToObject(entry, "type", type);
        
        // Get value based on type
        if (strcmp(type, "string") == 0 || strcmp(type, "password") == 0) {
            // Treat 'password' as 'string' (it's just a UI hint)
            char value_buf[256] = {0};
            ret = config_get_string(key, value_buf, sizeof(value_buf));
            if (ret == ESP_OK) {
                // Mask password fields
                if (strcmp(type, "password") == 0 || strstr(key, "pass") != NULL) {
                    cJSON_AddStringToObject(entry, "value", "********");
                } else {
                    cJSON_AddStringToObject(entry, "value", value_buf);
                }
            } else {
                cJSON_AddStringToObject(entry, "value", "");
            }
        }
        else if (strcmp(type, "integer") == 0) {
            int32_t value = 0;
            ret = config_get_int32(key, &value);
            if (ret == ESP_OK) {
                cJSON_AddNumberToObject(entry, "value", value);
            } else {
                cJSON_AddNumberToObject(entry, "value", 0);
            }
        }
        else if (strcmp(type, "boolean") == 0) {
            bool value = false;
            ret = config_get_bool(key, &value);
            if (ret == ESP_OK) {
                cJSON_AddBoolToObject(entry, "value", value);
            } else {
                cJSON_AddBoolToObject(entry, "value", false);
            }
        }
        
        cJSON_AddItemToArray(config_array, entry);
    }
    
    // Convert to string
    char *json_str = cJSON_PrintUnformatted(config_array);
    if (json_str == NULL) {
        ESP_LOGE(TAG, "Failed to serialize config JSON");
        cJSON_Delete(config_array);
        cJSON_Delete(schema);
        return ESP_ERR_NO_MEM;
    }
    
    *config_json = json_str;
    
    cJSON_Delete(config_array);
    cJSON_Delete(schema);
    
    ESP_LOGD(TAG, "Generated config JSON: %s", json_str);
    return ESP_OK;
}

esp_err_t config_set_all_from_json(const char *config_json) {
    if (config_json == NULL) {
        ESP_LOGE(TAG, "config_set_all_from_json: config_json is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Parse input JSON array
    cJSON *config_array = cJSON_Parse(config_json);
    if (config_array == NULL) {
        ESP_LOGE(TAG, "Failed to parse config JSON");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!cJSON_IsArray(config_array)) {
        ESP_LOGE(TAG, "Config JSON is not an array");
        cJSON_Delete(config_array);
        return ESP_ERR_INVALID_ARG;
    }
    
    // Parse schema for validation
    char *schema_str = NULL;
    esp_err_t ret = config_get_schema_json(&schema_str);
    if (ret != ESP_OK) {
        cJSON_Delete(config_array);
        return ret;
    }
    
    cJSON *schema = cJSON_Parse(schema_str);
    if (schema == NULL) {
        ESP_LOGE(TAG, "Failed to parse schema JSON");
        cJSON_Delete(config_array);
        return ESP_ERR_INVALID_ARG;
    }
    
    cJSON *schema_fields = cJSON_GetObjectItem(schema, "parameters");
    if (!cJSON_IsArray(schema_fields)) {
        ESP_LOGE(TAG, "Schema missing 'parameters' array");
        cJSON_Delete(schema);
        cJSON_Delete(config_array);
        return ESP_ERR_INVALID_ARG;
    }
    
    // Process each config entry
    cJSON *entry = NULL;
    int update_count = 0;
    cJSON_ArrayForEach(entry, config_array) {
        cJSON *key_item = cJSON_GetObjectItem(entry, "key");
        cJSON *type_item = cJSON_GetObjectItem(entry, "type");
        cJSON *value_item = cJSON_GetObjectItem(entry, "value");
        
        if (!cJSON_IsString(key_item) || !cJSON_IsString(type_item)) {
            ESP_LOGW(TAG, "Skipping entry with missing key or type");
            continue;
        }
        
        const char *key = key_item->valuestring;
        const char *type = type_item->valuestring;
        
        // Validate key exists in schema
        bool key_found = false;
        cJSON *schema_field = NULL;
        cJSON_ArrayForEach(schema_field, schema_fields) {
            cJSON *schema_key = cJSON_GetObjectItem(schema_field, "key");
            cJSON *schema_type = cJSON_GetObjectItem(schema_field, "type");
            
            if (cJSON_IsString(schema_key) && strcmp(schema_key->valuestring, key) == 0) {
                key_found = true;
                
                // Validate type matches
                if (cJSON_IsString(schema_type) && strcmp(schema_type->valuestring, type) != 0) {
                    ESP_LOGW(TAG, "Type mismatch for key '%s': expected %s, got %s",
                            key, schema_type->valuestring, type);
                    key_found = false;
                }
                break;
            }
        }
        
        if (!key_found) {
            ESP_LOGW(TAG, "Unknown or invalid key '%s', ignoring (forward compatibility)", key);
            continue;
        }
        
        // Set value based on type
        if (strcmp(type, "string") == 0 || strcmp(type, "password") == 0) {
            // Treat 'password' as 'string' (it's just a UI hint)
            if (!cJSON_IsString(value_item)) {
                ESP_LOGW(TAG, "Value for key '%s' is not a string", key);
                continue;
            }
            ret = config_set_string_no_commit(key, value_item->valuestring);
        }
        else if (strcmp(type, "integer") == 0) {
            if (!cJSON_IsNumber(value_item)) {
                ESP_LOGW(TAG, "Value for key '%s' is not a number", key);
                continue;
            }
            ret = config_set_int32_no_commit(key, (int32_t)value_item->valuedouble);
        }
        else if (strcmp(type, "boolean") == 0) {
            if (!cJSON_IsBool(value_item)) {
                ESP_LOGW(TAG, "Value for key '%s' is not a boolean", key);
                continue;
            }
            ret = config_set_bool_no_commit(key, cJSON_IsTrue(value_item));
        }
        else {
            ESP_LOGW(TAG, "Unknown type '%s' for key '%s'", type, key);
            continue;
        }
        
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set '%s': %s", key, esp_err_to_name(ret));
            cJSON_Delete(schema);
            cJSON_Delete(config_array);
            return ret;
        }
        
        update_count++;
    }
    
    // Commit all changes atomically
    ret = config_commit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit config changes: %s", esp_err_to_name(ret));
        cJSON_Delete(schema);
        cJSON_Delete(config_array);
        return ret;
    }
    
    cJSON_Delete(schema);
    cJSON_Delete(config_array);
    
    ESP_LOGI(TAG, "Updated %d configuration parameters", update_count);
    return ESP_OK;
}
