# ESP32 Project Template - GitHub Copilot Instructions

> **Note**: These instructions are for GitHub Copilot coding agent to help with code generation, issue resolution, and pull request improvements in this repository.

You are working on an ESP32 project template designed for rapid prototyping and production applications. This template provides a complete development environment with GitHub Codespaces, QEMU emulation, and example components for common IoT patterns.

## Project Context

### Template Purpose

This is a **template repository** designed to be:

- **Forked for new projects** - Starting point for ESP32 applications
- **Zero-setup development** - GitHub Codespaces with ESP-IDF pre-configured
- **Hardware optional** - QEMU emulation for testing without physical devices
- **Production-ready structure** - Component-based architecture following ESP-IDF best practices
- **Documentation included** - Sphinx-Needs requirements/design methodology

### Hardware Stack (When Using Real Hardware)

- **Microcontroller**: ESP32 (any variant supported by ESP-IDF)
- **Development Framework**: ESP-IDF v5.4.1
- **User adds their own**: Sensors, displays, or other peripherals

### Software Architecture

- **Component-based design**: Modular components in `main/components/`
- **Example components provided**:
  - `config_manager`: NVS configuration storage patterns
  - `web_server`: HTTP server with captive portal
  - `cert_handler`: HTTPS certificate handling (WIP)
  - `netif_uart_tunnel`: QEMU network bridge
- **Minimal main.c**: Template entry point users customize
- **Real-time OS**: FreeRTOS for task management
- **Memory optimized**: 4MB flash configuration

### Key Files and Components

- `main/main.c`: Application entry point (users customize this!)
- `main/components/config_manager/`: Example NVS configuration management
- `main/components/web_server/`: Example HTTP server with captive portal
- `main/components/cert_handler/`: HTTPS certificate handling (TODO: fix HTTPS)
- `main/components/netif_uart_tunnel/`: QEMU network bridge (required for emulation)

### Documentation Structure

- `docs/11_requirements/`: Sphinx-Needs requirements – **read these before implementing features**
- `docs/12_design/`: Component design specifications – **read the relevant spec before writing code**
- `docs/21_api/`: Auto-generated API docs per component
- `docs/90_guides/`: Developer guides (QEMU, flashing, switching dev modes, etc.)

> **Rule**: If a behaviour is specified in a requirements or design document, implement exactly that.
> Do **not** invent behaviour from general knowledge. When in doubt, check the relevant `.rst` files first and then consult with the user

## Development Guidelines

### Coding Standards

[ESP32 Coding Standards](./prompt-snippets/esp32-coding-standards.md)

### Build Instructions

[Detailed Build Instructions](./prompt-snippets/build-instructions.md)

### Development Workflow

[Development Process](./prompt-snippets/development.md)

### Commit Message Format

[Commit Message Guidelines](./prompt-snippets/commit-message.md)

## Project-Specific Instructions

### Memory Management

- **Always consider ESP32 memory constraints** when suggesting code changes
- **Use FreeRTOS-specific memory functions** (`heap_caps_malloc`, `heap_caps_free`)
- **Check available heap** before allocating large buffers
- **Prefer stack allocation** for small, temporary variables
- **Use IRAM_ATTR** sparingly and only when necessary

### API Documentation – Always Use Context7 First

Before writing or suggesting any FreeRTOS or ESP-IDF API calls, **always resolve the latest documentation via the context7 MCP server**:

- **FreeRTOS API**: use context7 library ID `/freertos/freertos`
- **ESP-IDF v5.4 API**: use context7 library ID `/websites/espressif_projects_esp-idf_en_v5_4_3_esp32c6`

This ensures correct function signatures, return types, and parameters for the exact versions used in this project (ESP-IDF v5.4.1, FreeRTOS as bundled).

### ESP-IDF Best Practices

- **Use component-based architecture** for new functionality
- **Include proper error handling** with ESP_ERROR_CHECK() and ESP_LOG functions
- **Follow ESP-IDF naming conventions** (esp_, ESP_, CONFIG_)
- **Use appropriate log levels** (ESP_LOGI, ESP_LOGW, ESP_LOGE, ESP_LOGD)
- **Handle WiFi events properly** using event handlers

### Hardware-Specific Considerations

- **Always document GPIO pin assignments** in the component or config header
- **Check ESP32 strapping pins** to avoid boot mode conflicts
- For all other hardware behaviour (timing, power, peripheral constraints): consult the relevant design spec in `docs/12_design/`

### Template-Specific Guidelines

- **Keep main.c minimal**: Application entry point should be simple and well-documented
- **Example components**: Maintain as reference implementations, not project code
- **Documentation**: Keep generic and applicable to any ESP32 project
- **QEMU support**: Ensure emulation works for testing without hardware
- **Codespaces first**: Prioritize GitHub Codespaces workflow over local setup

### Requirements Engineering

- **Methodology**: Follow Sphinx-Needs requirements engineering methodology
- **Requirement IDs**: Use format `REQ_<AREA>_<NUMBER>` (e.g., `REQ_CFG_1`, `REQ_WEB_3`)
- **Traceability**: Maintain bidirectional traceability from requirements → design → implementation → tests
- **Location**: Requirements stored in `docs/11_requirements/` with `.rst` format
- **Current Focus**: System, web server, config manager, and network tunnel requirements
- **Implementation Links**: Code must reference specific requirement IDs in comments for traceability

## Template Features

### Ready to Use

- ✅ Minimal application template in `main/main.c`
- ✅ Component-based architecture examples
- ✅ GitHub Codespaces with ESP-IDF pre-configured
- ✅ QEMU emulation with network support
- ✅ GDB debugging in VS Code
- ✅ Pre-commit hooks for quality gates
- ✅ Sphinx documentation with GitHub Pages
- ✅ Sphinx-Needs requirements/design structure

### Example Components (Optional)

- ✅ Configuration manager (NVS storage patterns)
- ✅ Web server (HTTP with captive portal)
- ✅ Network bridge (QEMU UART tunnel)
- 🚧 Certificate handler (HTTPS - needs fixing)

### Known Limitations

- 🚧 HTTPS not working in QEMU - HTTP works fine
- ℹ️ QEMU requires HTTP proxy for web access from host
- ℹ️ Codespaces only - local dev container not officially supported

## Important Notes

### When Suggesting Code Changes

1. **Check requirements first** – read `docs/11_requirements/` for the relevant area before writing code
2. **Check the design spec** – read `docs/12_design/` for the component before implementing
3. **Maintain component-based architecture** – create new components instead of cluttering `main.c`
4. **Use proper ESP-IDF error handling** and logging (`ESP_ERROR_CHECK`, `ESP_LOG*`)
5. **Reference requirement IDs in comments** – e.g. `/* REQ_WEB_3 */` for traceability
6. **Test in QEMU** when possible before suggesting hardware-specific code

### When Users Ask About Template Usage

1. **Guide them to customize main.c** – this is their entry point
2. **Point to example components** – show how to structure their code
3. **Explain QEMU workflow** – test without hardware first (see `docs/90_guides/switching-dev-modes.rst`)
4. **Recommend Codespaces** – best development experience
5. **Reference Sphinx-Needs docs** – for requirements/design methodology

## Quality Gates for Coding Agent

### Pre-commit Quality Checks

This project uses automated quality gates to ensure documentation and code quality. All changes **must pass pre-commit checks** before merging.

**Environment Setup**: The `.github/actions/setup-coding-agent-env/action.yml` ensures all required tools are available in CI.

#### Required Tools

When working on documentation changes, ensure these tools work:

1. **markdownlint-cli**: Markdown syntax and style validation
2. **sphinx**: Documentation build system with Sphinx-Needs
3. **pre-commit**: Automated hook management

#### How to Pass Quality Checks

**If you encounter CI failures:**

1. **Review the GitHub Actions log** for specific errors
2. **Run pre-commit locally** to see detailed output:

   ```bash
   pre-commit run --all-files --show-diff-on-failure
   ```

3. **Fix common issues:**
   - Add blank lines before/after lists and code blocks
   - Remove trailing whitespace
   - Fix broken links in documentation
   - Ensure proper heading hierarchy

4. **Commit fixes** and push - CI will re-run automatically

#### Quality Check Workflow

```text
Your Changes → Commit → Push → PR → CI runs pre-commit
                                    ↓
                              Pass? → ✅ Ready for review
                                    ↓
                              Fail? → ❌ Fix issues → Push fixes → CI re-runs
```

#### Documentation Standards

When modifying Markdown files:

- **Always add blank lines** around lists, code blocks, and headings
- **Use fenced code blocks** with language specifiers: ` ```yaml `
- **Validate links** before committing
- **Test Sphinx build** locally: `sphinx-build -b html docs docs/_build/html`

#### If Tools Are Not Available

If pre-commit tools are not available in your environment:

1. **Document in PR description** that manual quality check is required
2. **Request maintainer to run checks** locally or in CI
3. **Review CI output** carefully and address all reported issues

**Note:** The CI pipeline is the **authoritative quality gate**. All PRs must pass CI checks before merging, regardless of local environment setup.

## Build Environment

This template is designed for **GitHub Codespaces** and **VS Code Dev Containers**:

- **Primary**: GitHub Codespaces (zero-setup cloud development)
- **Alternative**: Local dev container (requires Docker)
- **Build commands**: Use `idf.py build` in the integrated terminal
- **ESP-IDF**: Pre-configured and ready to use (v5.4.1)

All build tools and dependencies are automatically available in the container environment.
```
