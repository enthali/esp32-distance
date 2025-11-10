Known Issues and Limitations
=============================

This document tracks known issues, limitations, and areas requiring improvement in this ESP32 project.

HTTPS Support
-------------

**Status:** Not yet implemented

**Current Behavior:**

- HTTP web server works fine in both QEMU and hardware
- HTTPS/SSL/TLS support is not implemented
- Certificate handler component exists but is work-in-progress

**Impact:**

- Web interface is not encrypted (HTTP only)
- Credentials transmitted in plain text over WiFi
- Not suitable for production deployment without VPN/secure network

**Recommendation:**

- Use only on trusted networks
- For production: Implement HTTPS before deployment
- Certificate handler component provides foundation for future HTTPS support

**Workaround:** Deploy only on secure/isolated networks


DNS Captive Portal
------------------

**Issue:** DNS server for captive portal detection is not implemented.

**Current Behavior:**

- Device runs as WiFi AP in configuration mode
- No automatic captive portal popup on mobile devices
- Users must manually navigate to ``http://192.168.4.1``

**Root Cause:**

- DNS server not implemented
- Captive portal detection varies by platform (iOS/Android/Windows)

**Impact:**

- Reduced user experience in AP mode configuration
- Users must manually discover device IP address (``192.168.4.1``)

**Status:** Open - low priority

**Workaround:** Document explicit IP address (``http://192.168.4.1``) in user documentation


Contributing
------------

If you encounter additional issues or have solutions to existing ones:

1. Check if issue already documented here
2. Report via GitHub Issues
3. Provide detailed reproduction steps
4. Include ESP-IDF version, hardware variant, build configuration

----

*Last Updated: November 2025*
