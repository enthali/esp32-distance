# Build & Flash Instructions (ESP32)

This guide is intentionally generic — adapt it to the reference repository (PlatformIO or ESP-IDF).

Note: At the start of the workshop we will demonstrate a live build and flash of `main` to a demo ESP32.

1. Preparations

- Ensure ESP32 drivers are installed (if required)
- Know the device port (e.g., COM3 on Windows)
- PlatformIO installed in the Codespace or ESP-IDF toolchain available

1. Standard steps (PlatformIO)

- Open a terminal
- `git checkout main`
- `git pull origin main`
- `pio run --target upload -e <environment>`  # pick `<environment>` from `platformio.ini`
- `pio device monitor`  # view serial output

1. Standard steps (ESP-IDF)

- Open a terminal
- `git checkout main`
- `git pull origin main`
- `idf.py set-target esp32`  # optional
- `idf.py -p <PORT> flash monitor`

1. Notes for the demo

- We will build the release from `main` and flash it to a demo ESP32 to show the complete process.
- If build/upload fails: check COM port, cable, board drivers and the devcontainer environment.

1. Quick checklist (for participants)

- [ ] Host detects the board
- [ ] Tools (PlatformIO / IDF) are available
- [ ] `git` configured (user.name / user.email)
- [ ] Build succeeded
- [ ] Flash succeeded
- [ ] Serial monitor shows expected logs

---

Next step: prepare the Issue / MCP exercise.
