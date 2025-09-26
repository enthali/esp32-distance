# Embedded GitHub Coding Workshop - Brainstorming

Goal: Hands-on workshop with an ESP32, an ultrasonic sensor (e.g., HC-SR04), and an addressable LED strip (e.g., WS2812 / NeoPixel). We'll use GitHub Codespaces and GitHub Copilot / GitHub MCP (coding agent) to change the LED display based on sensor readings and observe automated changes performed by a coding agent.

## Introduction (short)
- Hardware: ESP32, ultrasonic sensor, WS2812 LED strip.
- Software: Visual Studio Code / Codespaces, GitHub account, GitHub Copilot, GitHub MCP (coding agent), PlatformIO or ESP-IDF as build system.
- Workshop flow: Demo build & flash on main → Fork & Codespace → Read requirements → Design & write prompts → Create issue (via Copilot prompt) → Assign coding agent → Agent works → Check branch, build, test.

## Detailed schedule
1. Welcome & goals (5 min)
2. Hardware & software overview (5 min)
3. Live demo: build and flash the current `main` to an ESP32 (10 min)
   - Goal: show the build/flash process and confirm the base setup works.
4. Accounts & repo setup (10 min)
   - Participants sign in with their personal GitHub accounts.
   - Navigate to the reference project and fork the repository.

4a. Participants: First personal build & flash (10 min)
   - Goal: Each participant performs the first build/flash of `main` locally or in their Codespace.
   - Trainer assists with port/environment issues.
5. Open Codespace (together, step-by-step) (10–15 min)
6. Read requirements: `docs/requirements` (10 min)
   - We look at how requirements are structured.
7. Design thinking & prompt creation (15–20 min)
   - Participants decide how they want to change the LED display.
   - Write a prompt that instructs GitHub Copilot to create an issue asking the coding agent to:
     - update the requirement,
     - update files in `docs/design`,
     - change the code accordingly,
     - and ensure traceability (Requirements → Design → Code).
8. Create issue via Copilot prompt + GitHub MCP (5 min)
   - Create the issue using GH MCP or Copilot-generated text.
9. Assign & observe the coding agent (10 min)
   - Assign the agent either via GitHub Web UI or via GH MCP.
   - Short break / group discussion / coffee (10 min) while the agent works.
10. Review & merge (20 min)
   - The agent creates a branch/PR; we check out the branch, build the project and test on the ESP32.
   - Discussion: what went well / problems / improvements.

## Display requirements (preparation)
- We point participants to `docs/requirements` in the reference repo.
- Expectation: structured requirement documents (id, owner, description, acceptance criteria, mapping to design id).

## Prompt template (example)
Participants should adapt the prompt to their case. Example prompt:

"Please create an issue in my forked repository that describes the following task:

- Change requirement <REQUIREMENT_ID>: <new description / acceptance criteria>
- Update the design documentation in `/docs/design` so the change is traceable (include mapping from requirement ID to design changes).
- Modify the code so the LED animation matches the new requirement.
- Ensure traceability: link Requirement -> Design -> Code changes in the issue and in the modified files.

Also create a PR branch and instruct the GitHub Coding Agent to perform the changes automatically and include the related IDs in commit messages."

## Issue template (for the exercise)
- Target: requirement ID
- Description (new)
- Acceptance criteria
- Mockup / animation idea
- Priority
- Desired branch name

## Build & flash (short guide)
- We use PlatformIO / ESP-IDF (depending on the reference repo).
- Steps:
  1. `git checkout main`
  2. `git pull origin main`
  3. Build (PlatformIO: `pio run --target upload` / ESP-IDF: `idf.py -p <PORT> flash`)
  4. Monitor (PlatformIO: `pio device monitor` / ESP-IDF: `idf.py -p <PORT> monitor`)

## Tests & acceptance
- Functional: LED shows expected color coding based on distance.
- Traceability: links from Requirement -> Design -> Code exist.

## Discussion topics
- What are a coding agent's limitations?
- How can we automate traceability?
- Security and review processes for agent-made changes.

---

File created as a base for the workshop. Next step: Copilot prompt templates.
