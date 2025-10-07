# Commit Message Guidelines

## Format

```xml
<type>(<scope>): <subject>

<body>

<footer>
```

## Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting, etc.)
- **refactor**: Code refactoring without functionality changes
- **test**: Adding or modifying tests
- **chore**: Build process, dependency updates, etc.
- **perf**: Performance improvements
- **security**: Security improvements

## ESP32 Project Specific Scopes

- **sensor**: Distance sensor related changes
- **led**: LED controller changes
- **wifi**: WiFi manager and networking
- **web**: Web server and interface
- **build**: Build system and configuration
- **memory**: Memory optimization
- **https**: HTTPS and security implementation
- **component**: Component architecture changes

## Examples

### Feature Addition

```text
feat(https): Implement certificate generation and embedding

- Add CMake script for automated certificate generation
- Embed self-signed certificates using ESP-IDF EMBED_FILES
- Configure 10-year certificate validity for device lifecycle
- Integrate OpenSSL tools in build process
```

### Bug Fix

```text
fix(sensor): Resolve HC-SR04 timeout handling

- Add proper timeout validation for echo pin
- Implement retry logic for failed readings
- Log sensor communication errors appropriately
- Tested with various distance ranges
```

### Memory Optimization

```text
perf(memory): Optimize flash configuration for 4MB modules

- Updated sdkconfig to use 4MB flash size
- Switched to Single App Large partition table
- Increased available flash from 14% to 41% free space
- Prepared build configuration for HTTPS implementation
```

### Documentation

```text
docs(build): Update build instructions with flash configuration

- Document 4MB flash memory configuration
- Add memory usage verification commands
- Include menuconfig navigation instructions
- Prepare documentation for HTTPS readiness
```

### Component Refactoring

```text
refactor(component): Extract WiFi manager monitoring logic

- Move WiFi connection monitoring to wifi_manager.c
- Implement wifi_manager_monitor() function
- Update main.c to use new monitoring API
- Maintain backward compatibility
```

## Subject Line Rules

- Use imperative mood ("Add" not "Added")
- No period at the end
- Maximum 50 characters
- Capitalize first letter

## Body Guidelines

- Explain what and why, not how
- Use bullet points for multiple changes
- Reference issue numbers when applicable
- Include testing notes for critical changes

## Footer

- Reference related issues: `Closes #123`
- Note breaking changes: `BREAKING CHANGE: ...`
- Co-author attribution: `Co-authored-by: Name <email>`
