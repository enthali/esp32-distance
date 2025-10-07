# ESP32 Distance Project - Feature Intentions

This document contains **unnumbered** feature intentions that may or may not be implemented. These are flexible ideas that can be reordered, modified, or moved to the planned roadmap as priorities become clear.

---

## Workshop (1.5h): GitHub Codespaces & Copilot Demo 🛠️ **(currently in development)**

- **Introduction & Setup**: Quick overview of the ESP32 Distance Project and Codespaces environment.
- **Codespaces Walkthrough**: Launching, configuring, and navigating a Codespace for embedded development. Compiling for emulation and target device
- **Copilot in Action**: Live coding session—using Copilot to scaffold ESP32 components and documentation.
- **Collaborative Coding**: Pair programming and code review using Codespaces and Copilot suggestions.
- **Q&A and Best Practices**: Discussing workflow tips, troubleshooting, and integrating Copilot into daily development.

## HTTPS Security Implementation 💭 **on Hold**

- 💭 **HTTPS Server**: Replace HTTP with encrypted HTTPS using ESP32 SSL/TLS support
- 💭 **Self-Signed Certificates**: Generate and embed certificates for local IoT device use
- 💭 **Certificate Generation**: Build-time certificate creation and embedding
- 💭 **Mixed Mode Support**: HTTPS for production, HTTP fallback for development
- 💭 **Browser Compatibility**: Handle self-signed certificate warnings appropriately

## OTA Firmware Updates 💭 **INTENDED**

- **Over-The-Air Updates**: ESP32 OTA partition scheme and update mechanism
- **Version Management**: Firmware versioning and rollback capability
- **Update Server**: Simple HTTP/HTTPS server for firmware distribution
- **Security**: Signed firmware updates and secure boot
- **User Interface**: Web-based firmware update with progress indication

## Security Hardening 💭 **INTENDED**

- **WiFi Security**: WPA3 support and strong encryption
- **Web Interface Security**: HTTPS, session management, CSRF protection
- **Access Control**: Basic authentication for configuration pages
- **Network Security**: Firewall rules and secure communication
- **Credential Protection**: Encrypted storage of sensitive data

Each requirements document should use the `REQ-<AREA>-<NUMBER>` ID format and reference corresponding design documents for traceability.
