# Issue / GitHub MCP (Coding Agent) Exercise

Goal: Participants will create an issue via a Copilot prompt asking the coding agent to update requirements, design, and code and ensure traceability.

Preparations
- Make sure you're working in your forked repository.
- Ensure Copilot and GitHub MCP access are available.

Flow
1) Choose a requirement
- Open `docs/requirements` and pick a requirement that maps to an LED behavior change.

2) Adapt a prompt
- Use one of the templates from `COPILOT-PROMPTS.md` and replace placeholders.

3) Execute the prompt (create the issue)
- Option A: Use Copilot in Codespaces, enter the prompt into an editor buffer and let Copilot generate the issue text.
- Option B: Create the issue manually and paste the text.
- Important: the issue should include instructions for:
  - requirement change (text + acceptance criteria)
  - updating `docs/design`
  - concrete code changes or modules
  - traceability links

4) Assign the issue to the GitHub Coding Agent
- Web UI: open the issue -> Assignees -> add GitHub Coding Agent (or bot)
- Or use GH MCP to assign the agent via API

5) Observe and discuss
- Watch the changes the agent makes in the group.
- Take a short coffee / discussion break (~10 min)

6) Check the branch and test
- The agent will create a branch and open a PR.
- `git fetch` and `git checkout <agent-branch>`
- Build & flash, test on the ESP32

Evaluation criteria
- Functionality: LEDs behave as expected
- Traceability: Requirement → Design → Code links exist
- Commit quality and PR description

Trainer tips
- Prepare 1–2 demo issues in advance in case participants need help.
- Respect privacy: participants use their own accounts and forks.

---

Next step: create the workshop README and FAQs.
