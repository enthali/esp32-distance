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


## ESP32 Project Specific Scopes & Branch Naming

Use descriptive branch names and commit scopes:

- `feat/component-name` (component)
- `fix/issue-description` (fix)
- `docs/update-topic` (docs)
- `build` (build system)
- `config` (configuration management)
- `web` (web server)
- `network` (networking)
- `memory` (memory optimization)
- `requirements` (requirements docs)
- `design` (design docs)

## Examples

### Feature Addition

```text
feat(config): Implement configuration backup and restore

- Add NVS backup mechanism for configuration data
- Implement restore from backup on corruption detection
- Add validation of restored configuration
- Tested with power loss scenarios
```

### Bug Fix

```text
fix(network): Resolve WiFi reconnection timeout

- Add proper timeout validation for connection attempts
- Implement exponential backoff for retry logic
- Log connection errors appropriately
- Tested with various network conditions
```

### Memory Optimization

```text
perf(build): Optimize flash partition configuration

- Adjust partition table for optimal space usage
- Reduce bootloader size overhead
- Increase available application space
- Verified memory usage with idf.py size
```

### Documentation

```text
docs(requirements): Add system requirements documentation

- Document functional requirements in Sphinx-Needs format
- Add requirement IDs and traceability links
- Include acceptance criteria for each requirement
- Update requirements index
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

## See Also

- [Development Workflow](development.md) - Branch strategy and PR process
- [ESP32 Coding Standards](esp32-coding-standards.md) - Code conventions and documentation
- [Build Instructions](build-instructions.md) - Building and debugging
