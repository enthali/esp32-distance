# Codespaces & Repository Setup - Step-by-step

Goal: Have every participant sign in with their GitHub account, fork the reference repo, and open it in a Codespace so we can edit and test together.

Prerequisites for participants
- GitHub account (personal)
- Codespaces enabled for the account (or organization entitlement)
- GitHub Copilot (license may be required)
- Local hardware: ESP32, USB cable, ultrasonic sensor, LED strip, USB port

Steps

1) Fork the repository
- Navigate to the reference repository (share the link in the workshop chat)
- Click the "Fork" button in the top-right and wait for the fork to complete

2) Configure your fork (optional)
- Optionally add Secrets & Variables for Codespaces under Settings -> Secrets & variables -> Codespaces (e.g., API keys)

3) Create a Codespace
- On your fork: click "Code" -> "Open with Codespaces" -> "New codespace"
- Choose the recommended machine type (Standard)
- Wait until the Codespace initializes

4) Initialize your workspace
- Open a terminal inside the Codespace
- Optionally run: `git config user.email "you@example.com"` and `git config user.name "Your Name"`
- Create a working branch: `git checkout -b workspace-<yourname>`

5) Files & paths to inspect
- Open `docs/requirements` and `docs/design`
- Locate `src/` for LED and sensor logic

6) Run the live demo together
- We'll guide the participants through the demo steps and help with issues

Troubleshooting
- Codespaces not available: fallback to local VS Code + Dev Container or a local clone
- Build errors: check PlatformIO/ESP-IDF versions in `devcontainer.json` / repo README

---

Next step: add build & flash instructions.
