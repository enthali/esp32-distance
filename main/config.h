/**
 * @file config.h
 * @brief Centralized Configuration Header for ESP32 Distance Sensor Project
 * 
 * This header consolidates all user-configurable parameters and system-level
 * configuration values into a single location, eliminating scattered magic
 * numbers throughout the codebase.
 * 
 * SCOPE:
 * - User-configurable parameters affecting system behavior
 * - Network and timing parameters
 * - Default values for runtime configuration
 * 
 * EXCLUDED:
 * - Hardware timing specifications (WS2812 bit timing, etc.)
 * - ESP-IDF task stack sizes and priorities
 * - Protocol constants (HTTP status codes, etc.)
 * - Component-internal buffer sizes
 * 
 * @author ESP32 Distance Project Team
 * @date 2025
 * @version 1.0
 * 
 * Requirements Traceability:
 * - REQ-CFG-1: Centralized Configuration Header
 * - REQ-CFG-2: Use of Centralized Configuration
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DISTANCE SENSOR CONFIGURATION (User Configurable)
// =============================================================================

/**
 * @brief The distance mapped to the first LED (mm)
 * @note Range: 50-1000 (5.0-100.0cm)
 * @requirement REQ-CFG-1 AC-1
 */
#define DEFAULT_DISTANCE_MIN_MM         100

/**
 * @brief The distance mapped to the last LED (mm) 
 * @note Range: 200-4000 (20.0-400.0cm), must be larger than DEFAULT_DISTANCE_MIN_MM
 * @requirement REQ-CFG-1 AC-1
 */
#define DEFAULT_DISTANCE_MAX_MM         500    

/**
 * @brief How often to measure distance (milliseconds)
 * @note Range: 50-1000
 * @requirement REQ-CFG-1 AC-1
 */
#define DEFAULT_MEASUREMENT_INTERVAL_MS 100      

/**
 * @brief Maximum time to wait for ultrasonic echo (ms)
 * @note Range: 10-50, must be < measurement interval
 * @requirement REQ-CFG-1 AC-1
 */
#define DEFAULT_SENSOR_TIMEOUT_MS       30       

/**
 * @brief Ambient temperature for sound speed calculation (tenths of Celsius)
 * @note Range: -200 to 600 (-20.0 to 60.0°C), value 200 = 20.0°C
 * @requirement REQ-CFG-1 AC-1
 */
#define DEFAULT_TEMPERATURE_C_X10       200

/**
 * @brief Exponential moving average smoothing factor
 * @note Range: 100-1000, where 1000=1.0 (no smoothing), 300=0.3 (balanced), 100=0.1 (heavy smoothing)
 * @requirement REQ-CFG-1 AC-1
 */
#define DEFAULT_SMOOTHING_FACTOR        300     

// =============================================================================
// LED CONTROLLER CONFIGURATION (User Configurable)
// =============================================================================

/**
 * @brief Number of LEDs in the strip
 * @note Range: 1-60
 * @requirement REQ-CFG-1 AC-2
 */
#define DEFAULT_LED_COUNT               40       

/**
 * @brief LED brightness level
 * @note Range: 10-255 (0=off, 255=max brightness)
 * @requirement REQ-CFG-1 AC-2
 */
#define DEFAULT_LED_BRIGHTNESS          128      

// =============================================================================
// WIFI CONFIGURATION (User Configurable)
// =============================================================================

/**
 * @brief WiFi access point channel
 * @note Range: 1-13
 * @requirement REQ-CFG-1 AC-3
 */
#define DEFAULT_WIFI_AP_CHANNEL         1        

/**
 * @brief Maximum simultaneous AP connections
 * @note Range: 1-10
 * @requirement REQ-CFG-1 AC-3
 */
#define DEFAULT_WIFI_AP_MAX_CONN        4        

/**
 * @brief Station connection retry attempts (deprecated - restart-based approach)
 * @note Legacy parameter, new simplified approach uses restart instead of retries
 * @requirement REQ-CFG-1 AC-3
 */
#define DEFAULT_WIFI_STA_MAX_RETRY      1        

/**
 * @brief Station connection timeout (milliseconds) - simplified approach
 * @note 10 seconds timeout before switching to AP mode via restart
 * @requirement REQ-CFG-1 AC-3
 */
#define DEFAULT_WIFI_STA_TIMEOUT_MS     (30 * 1000)  // 30 seconds for handshake issues

/**
 * @brief Auto-retry interval from AP mode back to STA (milliseconds)
 * @note 10 minutes auto-retry cycle for unattended operation
 * @requirement REQ-CFG-1 AC-3 
 */
#define DEFAULT_WIFI_AP_RETRY_MS        (10 * 60 * 1000)     

// =============================================================================
// HARDWARE PIN CONFIGURATION (System Level)
// =============================================================================

/**
 * @brief GPIO pin for LED data signal
 * @note Hardware specific, not runtime configurable
 */
#define LED_DATA_PIN                    GPIO_NUM_12

/**
 * @brief GPIO pin for distance sensor trigger
 * @note Hardware specific, not runtime configurable  
 */
#define DISTANCE_TRIGGER_PIN            GPIO_NUM_14

/**
 * @brief GPIO pin for distance sensor echo
 * @note Hardware specific, not runtime configurable
 */
#define DISTANCE_ECHO_PIN               GPIO_NUM_13

/**
 * @brief RMT channel for LED controller
 * @note Hardware specific, not runtime configurable
 */
#define LED_RMT_CHANNEL                 0

// =============================================================================
// WIFI NETWORK CONFIGURATION (System Level)
// =============================================================================

/**
 * @brief Default AP SSID when no configuration exists
 * @note System default, overridden by runtime configuration
 */
#define DEFAULT_WIFI_AP_SSID            "ESP32-Distance-Sensor"

/**
 * @brief Default AP password (empty = open network)
 * @note System default, overridden by runtime configuration
 */
#define DEFAULT_WIFI_AP_PASSWORD        ""

#ifdef __cplusplus
}
#endif