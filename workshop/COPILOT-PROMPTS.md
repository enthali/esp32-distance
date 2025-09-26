# GitHub Copilot / Coding Agent Prompt Templates

Purpose: Ready-to-use prompts participants can adapt to ask GitHub Copilot or a GitHub Coding Agent to create an issue. Each prompt includes placeholders participants must replace with their specific IDs/descriptions.

## 1) Create an issue: change requirement + ensure traceability

Template:

"Please create an issue in my forked repository with the title: '[REQ-<REQUIREMENT_ID>] Change: <short description>'.

In the issue body:
- Requirement ID: REQ-<REQUIREMENT_ID>
- New description: <full new requirement description>
- Acceptance criteria:
  - <AC1>
  - <AC2>
- Please update the design documentation under `/docs/design` so that:
  - There is a design ID that references REQ-<REQUIREMENT_ID>.
  - The design changes clearly describe how the LED animation follows the new requirement.
- Please modify the code (create a feature branch `feat/REQ-<REQUIREMENT_ID>-led`) so that:
  - The LED animation meets the acceptance criteria.
  - Commits are descriptive and reference `REQ-<REQUIREMENT_ID>` and `DES-<DESIGN_ID>`.
- Ensure that the issue, the design file, and code commits contain cross-references/links to guarantee traceability (Requirement → Design → Code).

Also instruct a GitHub Coding Agent to perform these changes and open a PR."

## 2) Design-only update (smaller scope)

Template:

"Create an issue: 'DES-<DESIGN_ID> Update for REQ-<REQUIREMENT_ID>' with the task:
- Update `/docs/design/DES-<DESIGN_ID>.md` with:
  - Mapping to requirement ID: REQ-<REQUIREMENT_ID>
  - Detailed description of the LED animation and the state machine
  - Test instructions and acceptance criteria
- Indicate which code modules should be changed (as references only).
- Suggest a branch name: `design/DES-<DESIGN_ID>-update`.

Please instruct the Coding Agent to complete the change and propose a PR."

## 3) Code-only change (concrete)

Template:

"Create an issue 'CODE-REQ-<REQUIREMENT_ID>: LED animation update' and do the following:
- Implement a new function `set_led_mode(mode)` in `src/led_controller.*` that supports three modes: `SAFE`, `NORMAL`, `ALERT`.
- Triggers: distance < 10cm => ALERT, 10–40cm => NORMAL, >40cm => SAFE.
- Ensure all new commits reference `REQ-<REQUIREMENT_ID>` and link the PR description to the requirement file `docs/requirements/REQ-<REQUIREMENT_ID>.md` and design file `docs/design/DES-<DESIGN_ID>.md`.
- Add tests (if present) or a test checklist in the PR description.

Please instruct the Coding Agent to apply the changes and open a PR."

## Notes for participants
- Replace placeholders (`<...>`) with real IDs/texts.
- Provide clear acceptance criteria — this helps the coding agent.
- Use exact file paths and module names from the reference repo.

---

Next step: create a step-by-step Codespaces guide.
