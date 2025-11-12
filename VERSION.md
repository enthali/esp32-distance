# Version Management

This project uses automatic build number incrementation.

## Version Format

```text
MAJOR.MINOR.BUILD
```

- **MAJOR**: Breaking changes (manually updated)
- **MINOR**: New features (manually updated)
- **BUILD**: Automatically incremented on each build

## How It Works

1. **VERSION file** in project root contains the current version
2. **CMakeLists.txt** reads VERSION and increments BUILD on each `idf.py build`
3. **FIRMWARE_VERSION** is available as C define in all components
4. **Documentation** automatically reads VERSION file

## Updating Version

### Increment Minor Version (New Feature)

```bash
echo "1.1.0" > VERSION
```

### Increment Major Version (Breaking Change)

```bash
echo "2.0.0" > VERSION
```

### Reset Build Number

The BUILD number is automatically incremented on each build. To reset:

```bash
echo "1.0.0" > VERSION
```

## Usage in Code

The firmware version is available as a compile-time define:

```c
#ifdef FIRMWARE_VERSION
    printf("Version: %s\n", FIRMWARE_VERSION);
#endif

// Individual components also available:
// VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD
```

## Current Version

Check current version:

```bash
cat VERSION
```

View version in build output:

```bash
idf.py reconfigure | grep "Project Version"
```
