# Workshop Exercises

The focus of these exercises is to utilize the GitHub Coding Agent. If you do not have access to the GitHub Coding Agent, feel free to use Agent Mode instead.

## Exercise A: Extend LED Patterns

Before starting, follow the steps in the hardware setup and getting started sections of the documentation to ensure your ESP32, HC-SR04 sensor, and WS2812 LED strip are properly connected and the firmware is flashed successfully. Make sure you have also completed the software installation steps in the Getting Started guide, so you can compile the project and run it on real hardware or in the emulator (see emulator setup).

### Prompt for Copilot Chat

Open the GitHub Copilot Chat window in your codespace and enter the following prompt:

```
I want you to create an issue on GitHub that we will then assign to the GitHub coding agent: We want to change the behavior of the system. Right now, when the measurement is below the minimum threshold, the first LED is red. However, I want every 10th LED to flash red with a 1-second frequency. Please update the requirements document ([requirements display](../requirements/display-requirements.md)), update and extend the design document ([design docs](../design/display-design.md)), and implement the solution.
```

This will guide Copilot Chat Agent to generate a GitHub issue using the GitHub MCP server, describing the new behavior, referencing the requirements and design documentation, and requesting both documentation and implementation updates.

---

### Assigning the Issue

Now you have two options to proceed:

1. **Assign via GitHub Website**  
    Go to your repository's "Issues" tab on GitHub, locate the newly created issue, and manually assign it to the GitHub Coding Agent (or the appropriate automation user/bot).

2. **Assign via Copilot Chat Prompt**  
    Alternatively, enter the following prompt in Copilot Chat to request assignment:

    ```
    Please assign the new issue to the GitHub coding agent.
    ```

Either method will ensure the issue is routed to the coding agent for implementation.

---

### Watching the Coding Agent's Pull Request

Once the issue is assigned, the GitHub Coding Agent will automatically begin work by creating a new pull request (PR). To observe the process:

1. **Navigate to the Pull Requests tab** in your repository on GitHub.
2. **Locate the new PR** created by the Coding Agent—its title will reference the issue and describe the requested behavior change.
3. **Monitor the PR timeline**:
     - The agent will set up the development environment.
     - Updates will be made to the requirements document (`display-requirements.md`), design document (`display-design.md`), and relevant code.
     - The agent will commit and push changes, referencing requirement IDs for traceability.
     - Automated or manual tests will be run to verify the new behavior.
     - The agent will request your review when the PR is ready.

This process typically takes 10–15 minutes. You can follow progress in real time by viewing the PR's conversation, commits, and checks tabs. Once notified, review the changes and provide feedback or approve the PR as needed.

---

### Testing and Reviewing the Solution

Back in VS Code, you can check out the GitHub Coding Agent's branch, build the project, and—fingers crossed—flash it to your device for testing.

Be sure to review the updated requirements and design documents. The Coding Agent has established bidirectional traceability from requirements to design to code and back, ensuring every change is documented and linked.

You may wonder why this is the case, as there was nothing in the prompt explicitly asking Copilot to do so.

Well, there is:

All implementation and documentation changes must follow the project's ESP32 Coding Standards and commit message guidelines.  


Check them out in `.github/`.

Note that previously, there was no timing behavior for the output LEDs. The Coding Agent not only updated the LED pattern but also implemented a time-based flashing algorithm that does not interfere with the task's blocking call to receive the next measurement from the distance sensor. This ensures the new flashing behavior is robust and does not impact sensor responsiveness.

### Creative LED Pattern Ideas

Want to go beyond the default LED behavior? Try these creative extensions:

- **Distance-based color gradients:** Map measured distance to a color spectrum (e.g., green for close, yellow for mid-range, red for far).
- **Animated effects:** Make LEDs smoothly transition colors as the distance changes, or create a "wave" effect along the strip.
- **Disco mode:** Implement a flashing or cycling rainbow pattern, or have random LEDs blink in sync with a timer.
- **Threshold alerts:** Flash all LEDs rapidly if an object is too close or too far.
- **Custom patterns:** Design your own logic—such as alternating colors, pulsing brightness, or displaying a progress bar.

Feel free to experiment and implement your own ideas.

---

## Exercise B: Improve Distance Filtering and Timeouts

Not only single-file actions can be handled by the Coding Agent. This project previously used floating-point arithmetic, and with one issue and the Coding Agent implementing the change, the entire logic is now in fixed-point integer. Prefer floating point? Go ahead—describe what you want in an issue...

## Exercise C: LED Animation on the Website

Let me know who gets this done first!
