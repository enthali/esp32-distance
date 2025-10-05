# Known L## 🖥️ Web UI in Emulator: No Network Device
- **What happens?** The webserver initializes and runs in QEMU, but you can't access it from your browser.
- **Why?** The Espressif QEMU fork doesn't emulate network devices (`open_eth` is not available). The HTTP server can bind to a port inside QEMU, but there's no network hardware to send/receive packets. Serial console is currently the only I/O channel.
- **Will it be fixed?** 
  - **Short term**: We're planning a serial HTTP tunneling solution (see `docs/planning/Features-Serial-HTTP-Tunneling.md`) that will multiplex HTTP traffic over the serial connection.
  - **Long term**: Network device emulation would need to be added to Espressif's QEMU fork (consider filing an issue with them).
- **Current status**: The webserver code works perfectly on real hardware. In QEMU, you'll see successful initialization logs, but the webserver is not accessible via network.ations & Quirks 🚧

Welcome, brave explorer! Every project has its dragons—here are ours. If you run into these, don’t panic: you’re not alone, and sometimes it’s just how the (micro)chips fall.

## 📱 Captive Portal: Not-So-Captive
- **What happens?** When you connect to the ESP32’s WiFi, your phone/tablet might not be magically whisked away to the configuration page. Instead, you’ll need to open your browser and type `192.168.4.1`.
- **Why?** Modern devices are picky about captive portals, and our ESP32 is doing its best. Some phones just won’t auto-redirect—blame the phone, not the project!
- **Will it be fixed?** Probably not. It’s a common IoT quirk, and most users figure it out quickly.

## 🖥️ Web UI in Emulator: No Show
- **What happens?** If you’re running the project in an emulator, the web interface might be unreachable.
- **Why?** Networking in emulators is tricky, and not all features are supported.
- **Will it be fixed?** Maybe! We might open a GitHub issue for this. If you have ideas, PRs are welcome.

## 🦄 Other Oddities
- If you find a new quirk, let us know! We love a good bug hunt (and a good laugh).

---

*Remember: If everything worked perfectly, it wouldn’t be called a “project.”*
