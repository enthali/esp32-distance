/**
 * @file config.h
 * @brief Application-Specific Configuration Parameters
 * 
 * This file defines all configurable parameters for the ESP32 application.
 * Users should modify this file to add/remove parameters specific to their project.
 * 
 * The configuration system uses two separate tables for memory efficiency:
 * - UINT16 parameters: Numeric values (0-65535)
 * - STRING parameters: Text values (max 64 characters)
 * 
 * ADDING NEW PARAMETERS:
 * 1. Add enum entry to config_uint16_id_t or config_string_id_t
 * 2. Add corresponding entry to CONFIG_UINT16_PARAMS[] or CONFIG_STRING_PARAMS[]
 * 3. No changes needed to config_manager.c!
 * 
 * REQUIREMENTS TRACEABILITY:
 * - REQ_CFG_3: Configuration data structure
 * - REQ_CFG_6: Parameter validation
 * 
 * @author ESP32 Template Project
 * @date 2025
 */

#pragma once

#include "config_manager.h"  // Import struct definitions

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// UINT16 PARAMETERS
// =============================================================================

/**
 * @brief UINT16 parameter identifiers
 * 
 * Each enum value serves as:
 * - Array index into CONFIG_UINT16_PARAMS[] table
 * - Part of NVS storage key (e.g., CONFIG_LED_COUNT=0 → NVS key "u0")
 * 
 * WARNING: Do NOT reorder existing enums! This breaks NVS compatibility.
 * Always append new parameters before CONFIG_UINT16_COUNT.
 */
typedef enum {
    CONFIG_LED_COUNT,        ///< Number of LEDs in strip (example parameter)
    CONFIG_LED_BRIGHTNESS,   ///< LED brightness level 0-255 (example parameter)
    
    // Add your uint16 parameters here...
    // CONFIG_SENSOR_TIMEOUT,
    // CONFIG_MEASUREMENT_INTERVAL,
    
    CONFIG_UINT16_COUNT      ///< Sentinel value (do not use)
} config_uint16_id_t;

/**
 * @brief UINT16 parameter metadata table
 * 
 * Defines constraints and default values for all uint16 parameters.
 * Stored in Flash ROM (const), zero RAM cost.
 * 
 * Each entry specifies:
 * - min: Minimum allowed value (inclusive)
 * - max: Maximum allowed value (inclusive)
 * - default_val: Factory default value
 */
static const config_uint16_param_t CONFIG_UINT16_PARAMS[CONFIG_UINT16_COUNT] = {
    // LED Count: 1-100 LEDs, default 40
    // (Example parameter for demonstration, not connected to hardware)
    [CONFIG_LED_COUNT] = {
        .min = 1,
        .max = 100,
        .default_val = 40
    },
    
    // LED Brightness: 10-255, default 128 (50% brightness)
    // Min=10 prevents completely dim LEDs
    // (Example parameter for demonstration)
    [CONFIG_LED_BRIGHTNESS] = {
        .min = 10,
        .max = 255,
        .default_val = 128
    },
};

// =============================================================================
// STRING PARAMETERS
// =============================================================================

/**
 * @brief STRING parameter identifiers
 * 
 * Each enum value serves as:
 * - Array index into CONFIG_STRING_PARAMS[] table
 * - Part of NVS storage key (e.g., CONFIG_WIFI_SSID=0 → NVS key "s0")
 * 
 * WARNING: Do NOT reorder existing enums! This breaks NVS compatibility.
 * Always append new parameters before CONFIG_STRING_COUNT.
 */
typedef enum {
    CONFIG_WIFI_SSID,        ///< WiFi network name (functional parameter)
    CONFIG_WIFI_PASSWORD,    ///< WiFi network password (functional parameter)
    
    // Add your string parameters here...
    // CONFIG_DEVICE_NAME,
    // CONFIG_MQTT_BROKER_URL,
    
    CONFIG_STRING_COUNT      ///< Sentinel value (do not use)
} config_string_id_t;

/**
 * @brief STRING parameter metadata table
 * 
 * Defines constraints and default values for all string parameters.
 * Stored in Flash ROM (const), zero RAM cost.
 * 
 * Each entry specifies:
 * - min_len: Minimum string length (0 = empty string allowed)
 * - max_len: Maximum string length (excluding null terminator, max 64)
 * - default_val: Pointer to default string in Flash ROM
 */
static const config_string_param_t CONFIG_STRING_PARAMS[CONFIG_STRING_COUNT] = {
    // WiFi SSID: 1-32 characters (IEEE 802.11 standard)
    // Used by WiFi manager for network connectivity
    [CONFIG_WIFI_SSID] = {
        .min_len = 1,
        .max_len = 32,
        .default_val = "ESP32-AP"
    },
    
    // WiFi Password: 0-63 characters (WPA2 standard)
    // min_len=0 allows open networks (no password)
    // Used by WiFi manager for network authentication
    [CONFIG_WIFI_PASSWORD] = {
        .min_len = 0,
        .max_len = 63,
        .default_val = ""
    },
};

// =============================================================================
// COMPILE-TIME VALIDATION
// =============================================================================

// Ensure parameter tables are properly sized
_Static_assert(sizeof(CONFIG_UINT16_PARAMS) / sizeof(CONFIG_UINT16_PARAMS[0]) == CONFIG_UINT16_COUNT,
               "CONFIG_UINT16_PARAMS table size mismatch - did you forget to add an entry?");

_Static_assert(sizeof(CONFIG_STRING_PARAMS) / sizeof(CONFIG_STRING_PARAMS[0]) == CONFIG_STRING_COUNT,
               "CONFIG_STRING_PARAMS table size mismatch - did you forget to add an entry?");

#ifdef __cplusplus
}
#endif
