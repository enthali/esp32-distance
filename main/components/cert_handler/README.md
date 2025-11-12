# Certificate Handler Component

**Status:** 🚧 Work in Progress

## Purpose

Handles SSL/TLS certificate management for HTTPS web server functionality.

## Features

- Certificate storage and retrieval
- Certificate validation
- Self-signed certificate support

## Known Issues

⚠️ **HTTPS functionality is currently not working in QEMU**

- HTTP mode works correctly
- Certificate handling implementation needs debugging
- Use HTTP for development and testing

## Configuration

Certificates are embedded at build time from `build/certs/`:

- `server.crt` - Server certificate
- `server.key` - Private key
- `ca.crt` - CA certificate (optional)

## Usage

```c
#include "cert_handler.h"

// Initialize certificate handler
esp_err_t ret = cert_handler_init();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Certificate handler init failed");
}

// Get certificate data
const char* cert_data = cert_handler_get_server_cert();
const char* key_data = cert_handler_get_server_key();
```

## Dependencies

- `mbedtls` - TLS/SSL library
- `esp_http_server` - HTTP server component

## TODO

- [ ] Debug HTTPS functionality in QEMU
- [ ] Add certificate rotation support
- [ ] Implement certificate validation
- [ ] Add certificate expiry checks

## See Also

- [Web Server Component](../web_server/README.md)
- [Certificate Generation Tool](../../../tools/certificates/generate_cert.py)
