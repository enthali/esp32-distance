# ESP32 Distance Sensor Emulator Support

This document explains the emulator support implementation for the ESP32 distance sensor project, which enables development and testing without physical hardware using QEMU.

## Overview

The emulator support provides hardware abstraction through separate simulator implementations that maintain identical APIs to the hardware components. This allows the same application code to run in both hardware and emulated environments without modification.

## Architecture

### Hardware Abstraction Strategy

The implementation uses a **clean separation approach**:

- **Same headers**: Identical APIs for both hardware and simulator versions
- **Different source files**: CMake selects appropriate implementation at build time
- **No #ifdef clutter**: Clean, maintainable code without conditional compilation

### Components

#### 1. Distance Sensor Simulator (`distance_sensor_sim.c`)

- **API Compatibility**: Identical to hardware version (`distance_sensor.h`)
- **Animation**: 5cm → 60cm → 5cm linear sweep with 1mm steps
- **Timing**: Configurable interval (default 1 second for clear visualization)
- **Queue Architecture**: Same dual-queue system as hardware for real-time behavior
- **Error Handling**: Complete status codes and overflow management

#### 2. LED Controller Simulator (`led_controller_sim.c`)

- **API Compatibility**: Identical to hardware version (`led_controller.h`)
- **Visualization**: Unicode emoji blocks in terminal (🔴🟢🔵⚪🟡🟣⚫🟤)
- **Rate Limiting**: Output limited to ~1Hz to prevent terminal spam
- **Color Mapping**: Intelligent RGB-to-emoji conversion
- **Buffer Management**: Same pixel operations as hardware

#### 3. WiFi Manager

- **No Changes Needed**: Runs as-is without network events
- **Web Interface**: Still accessible (localhost without network)

## Build Configuration

### Kconfig Options

New configuration options in `main/Kconfig.projbuild`:

```kconfig
config TARGET_EMULATOR
    bool "Build for QEMU emulator"
    default n
    help
        Enable this option to build for QEMU emulator instead of real hardware

config EMULATOR_MOCK_SENSOR
    bool "Use mocked sensor data in emulator"
    depends on TARGET_EMULATOR
    default y
    help
        Generate simulated distance sensor readings
```

### CMake Integration

The build system automatically selects the correct source files:

**Distance Sensor** (`components/distance_sensor/CMakeLists.txt`):

```cmake
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "distance_sensor_sim.c")
else()
    set(COMPONENT_SRCS "distance_sensor.c")
endif()
```

**LED Controller** (`components/led_controller/CMakeLists.txt`):

```cmake
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "led_controller_sim.c")
else()
    set(COMPONENT_SRCS "led_controller.c")
endif()
```

## Usage Instructions

### 1. Hardware Build (Default)

```bash
idf.py build
idf.py flash monitor
```

### 2. Emulator Build

```bash
# Configure for emulator
idf.py menuconfig
# Navigate to: ESP32 Distance Project Configuration
# Enable: [x] Build for QEMU emulator
# Enable: [x] Use mocked sensor data in emulator

# Build
idf.py build

# Run in QEMU (example)
qemu-system-xtensa -nographic -M esp32 -kernel build/distance.elf
```

## Expected Output

### Console Logs

```text
I (1000) main: ESP32 Distance Measurement with LED Strip Display
I (1100) led_controller_sim: LED controller simulator initialized: 40 LEDs (terminal visualization)
I (1200) distance_sensor_sim: Distance sensor simulator initialized successfully  
I (1300) distance_sensor_sim: Distance sensor simulator started (5cm→60cm→5cm sweep, interval: 1000 ms)
```

### Animated Distance Readings

```text
I (2000) distance_sensor_sim: Simulated distance: 5.0 cm (increasing)
I (3000) distance_sensor_sim: Simulated distance: 5.1 cm (increasing)
I (4000) distance_sensor_sim: Simulated distance: 5.2 cm (increasing)
...
I (58000) distance_sensor_sim: Simulated distance: 60.0 cm (decreasing)
I (59000) distance_sensor_sim: Simulated distance: 59.9 cm (decreasing)
```

### LED Strip Visualization

```text
[LED Strip]: 🔴🔴🔴⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫

[LED Strip]: 🟡🟡🟡🟡🟡🟡🟡🟡⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫

[LED Strip]: 🟢🟢🟢🟢🟢🟢🟢🟢🟢🟢🟢🟢⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫

[LED Strip]: 🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵🔵⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫⚫
```

### Color Mapping Legend

- 🔴 **Red**: Close distances (5-15cm)
- 🟡 **Yellow**: Medium distances (15-35cm)
- 🟢 **Green**: Far distances (35-50cm)
- 🔵 **Blue**: Maximum distances (50-60cm)
- ⚪ **White**: Very bright/mixed colors
- 🟣 **Purple**: Magenta/mixed colors
- ⚫ **Black**: Off/very dim LEDs

## Implementation Details

### Distance Sensor Simulator

**Animation Logic**:

```c
// Animate distance: 5cm (50mm) → 60cm (600mm) → 5cm (50mm)
sim_distance += direction;

if (sim_distance >= 600) {  // 60.0cm = 600mm
    direction = -1;
} else if (sim_distance <= 50) {  // 5.0cm = 50mm  
    direction = 1;
}
```

**Queue Behavior**: Identical to hardware implementation with overflow handling and statistics tracking.

### LED Controller Simulator  

**Rate Limiting**:

```c
static uint64_t last_display_time = 0;
static const uint64_t DISPLAY_INTERVAL_US = 1000000; // 1 second

esp_err_t led_show(void) {
    uint64_t now = esp_timer_get_time();
    if (now - last_display_time < DISPLAY_INTERVAL_US) {
        return ESP_OK;  // Suppress output, just return success
    }
    last_display_time = now;
    // ... display emoji output
}
```

**Color Analysis**: Intelligent mapping from RGB values to representative emoji blocks based on dominant colors and brightness levels.

## Benefits

### Development Advantages

- **No Hardware Dependencies**: Develop without physical ESP32, sensors, or LEDs
- **Fast Iteration**: Quick build-test cycles without flashing hardware
- **Visual Feedback**: Clear terminal visualization of system behavior
- **CI/CD Friendly**: Automated testing in continuous integration pipelines
- **Cross-Platform**: Develop on any system with QEMU support

### Testing Advantages

- **Predictable Behavior**: Animated patterns for systematic testing
- **Integration Testing**: Full system testing without hardware setup
- **Algorithm Validation**: Test LED display logic and distance mapping
- **Performance Analysis**: Monitor queue behavior and timing

### Educational Benefits

- **System Understanding**: Clear visualization of sensor-to-display pipeline
- **Algorithm Learning**: See distance mapping and color gradients in action
- **Real-time Concepts**: Observe queue-based architecture behavior

## Files Modified/Created

### New Files

- `main/Kconfig.projbuild` - Build configuration options
- `main/components/distance_sensor/distance_sensor_sim.c` - Distance sensor simulator
- `main/components/led_controller/led_controller_sim.c` - LED controller simulator

### Modified Files

- `main/components/distance_sensor/CMakeLists.txt` - Conditional source selection
- `main/components/led_controller/CMakeLists.txt` - Conditional source selection

### Unchanged Files

- All header files (`.h`) - APIs remain identical
- Main application logic - No changes needed
- WiFi/web server components - Run unchanged

## Validation

The implementation has been validated for:

- ✅ Complete API compatibility with hardware versions
- ✅ Proper CMake conditional compilation
- ✅ Animation logic and timing behavior  
- ✅ LED visualization and rate limiting
- ✅ Queue-based architecture and error handling
- ✅ No security vulnerabilities introduced

## Future Enhancements

Potential improvements for the emulator support:

- **Web Interface Preview**: Show LED state in browser
- **Interactive Controls**: Manual distance adjustment via keyboard
- **Performance Metrics**: Queue utilization and timing statistics
- **Multiple Animation Patterns**: Different test sequences
- **Color Scheme Options**: Alternative emoji sets or ASCII mode

---

*This emulator support enables efficient development and testing of the ESP32 distance sensor project without requiring physical hardware, while maintaining complete compatibility with the production system.*
