/**
 * @file cert_handler.c
 * @brief Certificate management implementation for ESP32 Distance Sensor
 * 
 * Provides access to embedded SSL certificates generated during build.
 * Certificates are embedded as binary data using ESP-IDF EMBED_FILES.
 * 
 * @author ESP32 Distance Project
 * @date 2025
 */

#include "cert_handler.h"
#include "esp_log.h"
#include <string.h>

static const char* TAG = "CERT_HANDLER";

// External references to embedded certificate files
// These symbols are created by ESP-IDF EMBED_FILES feature
extern const uint8_t server_crt_start[] asm("_binary_server_crt_start");
extern const uint8_t server_crt_end[]   asm("_binary_server_crt_end");
extern const uint8_t server_key_start[] asm("_binary_server_key_start");
extern const uint8_t server_key_end[]   asm("_binary_server_key_end");
extern const uint8_t ca_crt_start[]     asm("_binary_ca_crt_start");
extern const uint8_t ca_crt_end[]       asm("_binary_ca_crt_end");

esp_err_t cert_handler_get_server_cert(const char** cert_data, size_t* cert_len)
{
    if (cert_data == NULL || cert_len == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for server certificate");
        return ESP_ERR_INVALID_ARG;
    }
    
    *cert_data = (const char*)server_crt_start;
    *cert_len = server_crt_end - server_crt_start;
    
    ESP_LOGD(TAG, "Server certificate: %zu bytes", *cert_len);
    return ESP_OK;
}

esp_err_t cert_handler_get_server_key(const char** key_data, size_t* key_len)
{
    if (key_data == NULL || key_len == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for server private key");
        return ESP_ERR_INVALID_ARG;
    }
    
    *key_data = (const char*)server_key_start;
    *key_len = server_key_end - server_key_start;
    
    ESP_LOGD(TAG, "Server private key: %zu bytes", *key_len);
    return ESP_OK;
}

esp_err_t cert_handler_get_ca_cert(const char** cert_data, size_t* cert_len)
{
    if (cert_data == NULL || cert_len == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for CA certificate");
        return ESP_ERR_INVALID_ARG;
    }
    
    *cert_data = (const char*)ca_crt_start;
    *cert_len = ca_crt_end - ca_crt_start;
    
    ESP_LOGD(TAG, "CA certificate: %zu bytes", *cert_len);
    return ESP_OK;
}

esp_err_t cert_handler_init(void)
{
    ESP_LOGI(TAG, "Initializing certificate management");
    
    // Verify server certificate is available
    size_t server_cert_len = server_crt_end - server_crt_start;
    if (server_cert_len == 0) {
        ESP_LOGE(TAG, "Server certificate not found in firmware");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Verify server private key is available
    size_t server_key_len = server_key_end - server_key_start;
    if (server_key_len == 0) {
        ESP_LOGE(TAG, "Server private key not found in firmware");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Verify CA certificate is available
    size_t ca_cert_len = ca_crt_end - ca_crt_start;
    if (ca_cert_len == 0) {
        ESP_LOGE(TAG, "CA certificate not found in firmware");
        return ESP_ERR_NOT_FOUND;
    }
    
    ESP_LOGI(TAG, "Certificates initialized successfully");
    ESP_LOGI(TAG, "  Server cert: %zu bytes", server_cert_len);
    ESP_LOGI(TAG, "  Server key:  %zu bytes", server_key_len);
    ESP_LOGI(TAG, "  CA cert:     %zu bytes", ca_cert_len);
    
    return ESP_OK;
}

esp_err_t cert_handler_get_info(char* info_buffer, size_t buffer_size)
{
    if (info_buffer == NULL || buffer_size == 0) {
        ESP_LOGE(TAG, "Invalid parameters for certificate info");
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t server_cert_len = server_crt_end - server_crt_start;
    size_t server_key_len = server_key_end - server_key_start;
    size_t ca_cert_len = ca_crt_end - ca_crt_start;
    
    int written = snprintf(info_buffer, buffer_size,
        "SSL Certificates:\n"
        "  Server Certificate: %zu bytes\n"
        "  Server Private Key: %zu bytes\n"
        "  CA Certificate: %zu bytes\n"
        "  Generated: Build-time (25-year validity)\n"
        "  Type: Self-signed for IoT device",
        server_cert_len, server_key_len, ca_cert_len);
    
    if (written >= (int)buffer_size) {
        ESP_LOGW(TAG, "Certificate info truncated");
        return ESP_ERR_INVALID_SIZE;
    }
    
    return ESP_OK;
}