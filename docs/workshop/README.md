
# Workshop: Embedded GitHub Coding (Materials)

This section contains the workshop materials. The goal is to give participants hands-on experience working on open-source embedded projects (ESP32 + HC-SR04 + WS2812) using GitHub, Codespaces and Copilot/Coding-Agents.

Included files (short overview):

- `WORKSHOP-AGENDA.md` — agenda, goals and schedule
- `CODESPACES-GUIDE.md` — step-by-step guide to open a Codespace / Dev Container
- `BUILD-FLASH-INSTRUCTIONS.md` — build and flash instructions (ESP-IDF)
- `ISSUE-MCP-EXERCISE.md` — exercise: create an issue & use a coding agent
- `COPILOT-PROMPTS.md` — example prompts for Copilot / coding agents


## Prerequisites

See [Workshop Prerequisites](prerequisites.md) for all required software, accounts, and hardware.

## Quick flow
1. **Fork the repository**  
    Start by forking the main repository to your own GitHub account. This gives you a personal copy where you can make changes freely.

2. **Open a Codespace from your fork**  
    In your forked repository, click the green "Code" button and select "Create codespace on main". This launches a cloud-based VS Code environment with all dependencies pre-installed.

3. **Familiarize yourself with the repository**  
    Explore the project structure and key components (see overview above and in the repo). Understand where main application logic, components, and documentation are located.

4. **Build the project**  
    Use the "Build" button at the very bottom of the VS Code window (see screenshot below). This compiles the firmware using ESP-IDF in the Codespace.
    ![VS Code Build Button](../assets/images/build-button.png)

5. **Flash the firmware**  
    After a successful build, click the "Flash" button (also at the bottom of VS Code; see screenshot). This uploads the firmware to your connected ESP32 board.
     ![VS Code Flash Button](../assets/images/flash-button.png)

6. **Monitor the device**  
    Once flashing is complete, click the "Monitor" button (bottom of VS Code) to open a serial monitor. This lets you observe device logs, sensor readings, and debug output in real time.
     ![VS Code Monitor Button](../assets/images/monitor-button.png)

7. **Experience the running system**  
    With the device running, observe the LED strip responding to distance measurements. Use your phone or laptop to connect to the ESP32's WiFi access point, open the captive portal, and access the web interface. You can experiment with connecting the ESP32 to your own network or adjust range settings via the web UI.

8. **Proceed to workshop exercises**  
    After exploring the live system, continue with the workshop exercises (e.g., extending LED patterns, improving distance measurement, testing HTTPS features).

9. **Use Copilot/Coding-Agents for assistance**  
    Leverage GitHub Copilot and Copilot Chat to help with code suggestions, explanations, and troubleshooting as you work through the tasks.

10. **Review, test, and merge**  
     After completing tasks, review your code, run tests, and follow the standard merge process (branch → PR → review → merge).


## FAQ / Troubleshooting (short)

Q: Codespaces not available?

A: Use local VS Code with the included Dev Container, or follow `CODESPACES-GUIDE.md`.

Q: Copilot returns unhelpful suggestions?

A: Provide more context in the prompt (e.g. REQ IDs, file paths, expected return values). Always review suggestions — never accept blindly.
Q: The coding agent made incorrect changes?

A: Stop the agent, inspect the branch, and fix changes manually. Use the PR review workflow (branch → PR → review → merge). In the worst case, delete the agent's branch and start over. **Note:** By default, the coding agent works from the `main` branch. If you want the agent to work on your own branch, you must explicitly specify your branch when assigning the issue. The coding agent will always create and work in its own branch—it will not make changes directly to your branch.



---

## Happy Coding!

We hope you enjoy the workshop and learn valuable skills for embedded development with ESP32, GitHub, and Copilot. If you get stuck, ask questions, collaborate with others, and make the most of the tools and resources provided.

Happy coding! 🚀

