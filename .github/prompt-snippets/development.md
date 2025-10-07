# Developer Workflow

This document outlines the development workflow for the ESP32 Distance Sensor project.

## Quick Start

For developers new to the project:

1. **Clone and setup**:

   ```bash
   git clone <repository-url>
   cd distance
   ```

2. **Build and test**:

   ```powershell
   cmd /c "cd /D c:\workspace\ESP32_Projects\distance && C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat && idf.py flash monitor"
   ```

## Development Branches

### Main Branches

- `main`: Stable release branch
- `develop`: Development integration branch

### Feature Branches

Use descriptive branch names:

- `feature/dns-server-extraction`
- `fix/build-compilation-errors`
- `enhancement/wifi-timeout-logic`

## Commit Guidelines

### Commit Message Format

```text
<type>: <description>

[optional body]

[optional footer]
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `build`: Build system or dependency changes

### Examples

```bash
git commit -m "fix: Add missing include for bool type in dns_server.h"
git commit -m "feat: Extract DNS server into separate module"
git commit -m "docs: Update build instructions for Windows PowerShell"
```

## Testing Strategy

### Pre-commit Checklist

- [ ] Code compiles without warnings
- [ ] All custom components build successfully
- [ ] Device boots and initializes properly
- [ ] WiFi manager connects successfully
- [ ] Web server responds correctly
- [ ] Distance sensor provides valid readings
- [ ] LED strip displays correctly

### Testing Commands

```bash
# Clean build test
idf.py fullclean build

# Flash and monitor test
idf.py flash monitor

# Component-specific build test
```bash
idf.py build --verbose
```

## Code Style

### C Code Guidelines

- Use ESP-IDF standard formatting
- Include descriptive function comments
- Use meaningful variable names
- Follow ESP32 naming conventions
- Add error handling for all ESP-IDF calls

### File Organization

```text
main/
├── main.c                 # Application entry point
├── wifi_manager.h/c       # WiFi management module
├── web_server.h/c         # HTTP server implementation
├── dns_server.h/c         # DNS server for captive portal
├── display_logic.h/c      # LED strip control logic
└── CMakeLists.txt         # Build configuration
```

## Debugging Workflow

### Common Issues and Solutions

1. **Build Errors**:
   - Check include paths in CMakeLists.txt
   - Verify component dependencies
   - Review ESP-IDF version compatibility

2. **Runtime Issues**:
   - Enable verbose logging
   - Use ESP-IDF debugging tools
   - Monitor heap and stack usage

3. **WiFi Issues**:
   - Check network credentials
   - Verify AP mode configuration
   - Monitor connection state transitions

### Debug Configuration

Enable debugging in `sdkconfig`:

```text
CONFIG_LOG_DEFAULT_LEVEL_DEBUG=y
CONFIG_LOG_MAXIMUM_LEVEL_DEBUG=y
```

## Release Process

### Version Management

Update version in:

- `CMakeLists.txt`
- Project documentation
- Release notes

### Release Checklist

- [ ] All tests pass
- [ ] Documentation updated
- [ ] Version numbers incremented
- [ ] Release notes prepared
- [ ] Tagged release created

## Tools and Environment

### Required Tools

- ESP-IDF v5.4.1+
- Git for version control
- VS Code with ESP-IDF extension (recommended)
- Serial monitor application

### Recommended Extensions

- ESP-IDF Extension for VS Code
- C/C++ Extension Pack
- GitLens for Git integration
- Markdown All in One

## Contributing

### Pull Request Process

1. Create feature branch from `develop`
2. Implement changes with tests
3. Update documentation if needed
4. Submit PR with detailed description
5. Address review feedback
6. Merge after approval

### Code Review Guidelines

- Focus on code correctness and style
- Verify build and runtime functionality
- Check for proper error handling
- Ensure documentation is updated
- Test on actual hardware when possible
