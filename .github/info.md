# GitHub Configuration

This directory contains GitHub-specific configuration files for the ESP32 Distance Sensor project.

## Contents

### Copilot Instructions

- **[copilot-instructions.md](copilot-instructions.md)** - Custom instructions for GitHub Copilot coding agent
  - Comprehensive project context and technical guidelines
  - ESP32 and ESP-IDF specific best practices
  - Hardware-specific considerations and constraints
  - Links to detailed prompt snippets in [`prompt-snippets/`](prompt-snippets/)

### Prompt Snippets

The [`prompt-snippets/`](prompt-snippets/) directory contains detailed reference documentation:

- **[esp32-coding-standards.md](prompt-snippets/esp32-coding-standards.md)** - ESP32/ESP-IDF coding conventions and patterns
- **[build-instructions.md](prompt-snippets/build-instructions.md)** - Build, flash, and troubleshooting guides
- **[development.md](prompt-snippets/development.md)** - Development workflow and branch management
- **[commit-message.md](prompt-snippets/commit-message.md)** - Commit message format and examples
- **[oft-requirements.md](prompt-snippets/oft-requirements.md)** - OpenFastTrack requirements methodology

### GitHub Actions

- **[actions/](actions/)** - Reusable GitHub Actions for CI/CD
  - `setup-coding-agent-env/` - Environment setup for coding agent and CI
- **[workflows/](workflows/)** - GitHub Actions workflow definitions
  - Pre-commit quality checks (linting, building, testing)
  - Automated CI/CD pipelines

## Using Copilot in This Repository

GitHub Copilot will automatically use the instructions in `copilot-instructions.md` when:

- Generating code suggestions in your IDE
- Resolving issues assigned to `@copilot`
- Reviewing pull requests
- Working on GitHub Copilot Workspace tasks

The instructions provide context about:

- Project architecture and hardware constraints
- Memory management for ESP32 (4MB flash)
- FreeRTOS and ESP-IDF best practices
- Testing, linting, and quality gates
- OpenFastTrack requirements methodology

## For Contributors

When contributing to this project:

1. Review the [copilot-instructions.md](copilot-instructions.md) to understand project standards
2. Use GitHub Copilot with confidence - it's configured for this project
3. Follow the quality gates outlined in the instructions
4. Reference the prompt snippets for detailed guidance on specific areas

## Maintenance

These instructions should be updated when:

- Project architecture changes significantly
- New development tools or practices are adopted
- Hardware or technical constraints change
- New team members need different context

For questions or suggestions about these instructions, open an issue or discussion.
