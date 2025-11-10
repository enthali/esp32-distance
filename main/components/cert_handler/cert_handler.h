/**
 * @file cert_handler.h
 * @brief Certificate management for ESP32 Distance Sensor HTTPS server
 * 
 * This module provides access to embedded SSL certificates generated during
 * the build process. Certificates are automatically generated with 25-year
 * validity for long-term IoT device deployment.
 * 
 * @author ESP32 Distance Project
 * @date 2025
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get embedded server certificate data
 * 
 * Returns a pointer to the embedded server certificate (PEM format)
 * that was generated during the build process.
 * 
 * @param[out] cert_data Pointer to certificate data (null-terminated)
 * @param[out] cert_len Length of certificate data including null terminator
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if parameters are NULL
 */
esp_err_t cert_handler_get_server_cert(const char** cert_data, size_t* cert_len);

/**
 * @brief Get embedded server private key data
 * 
 * Returns a pointer to the embedded server private key (PEM format)
 * that was generated during the build process.
 * 
 * @param[out] key_data Pointer to private key data (null-terminated)
 * @param[out] key_len Length of private key data including null terminator
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if parameters are NULL
 */
esp_err_t cert_handler_get_server_key(const char** key_data, size_t* key_len);

/**
 * @brief Get embedded CA certificate data
 * 
 * Returns a pointer to the embedded CA certificate (PEM format)
 * that was generated during the build process.
 * 
 * @param[out] cert_data Pointer to CA certificate data (null-terminated)
 * @param[out] cert_len Length of CA certificate data including null terminator
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if parameters are NULL
 */
esp_err_t cert_handler_get_ca_cert(const char** cert_data, size_t* cert_len);

/**
 * @brief Initialize certificate management
 * 
 * Verifies that all required certificates are available and properly
 * embedded in the firmware. This should be called during system
 * initialization before starting the HTTPS server.
 * 
 * @return ESP_OK if all certificates are available,
 *         ESP_ERR_NOT_FOUND if any certificates are missing
 */
esp_err_t cert_handler_init(void);

/**
 * @brief Get certificate information for logging
 * 
 * Provides certificate metadata for diagnostic and logging purposes.
 * Does not expose sensitive private key information.
 * 
 * @param[out] info_buffer Buffer to store certificate information
 * @param[in] buffer_size Size of the information buffer
 * @return ESP_OK on success, ESP_ERR_INVALID_SIZE if buffer too small
 */
esp_err_t cert_handler_get_info(char* info_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif