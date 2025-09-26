# Issue: Add Emulator Support with Hardware Abstraction

## Problem Statement
The ESP32 distance sensor project currently only builds for real hardware. We need to add emulator support (QEMU) to enable development and testing without physical hardware.

## Requirements

### Core Requirements  
- [ ] Kconfig option to switch between hardware and emulator builds
- [ ] Simulator components with identical APIs to hardware components
- [ ] QEMU emulator build and execution
- [ ] Visual LED strip feedback in terminal

### Components to Implement Simulators

#### 1. Distance Sensor Simulator
**File:** `components/distance_sensor/distance_sensor_sim.c`
- **Must implement complete API** from `distance_sensor.h`
- **Same API behavior:** Queue-based, blocking calls, identical return values
- **Animation:** 5cm → 60cm → 5cm linear sweep, 1mm steps, 1 second intervals

#### 2. LED Controller Simulator  
**File:** `components/led_controller/led_controller_sim.c`
- **Must implement complete API** from `led_controller.h`
- **Same API behavior:** Buffer management, pixel setting, identical return values
- **Visualization:** Unicode emoji blocks in terminal (🔴🟢🔵⚪🟡🟣⚫)
- **Rate limiting:** Only output to terminal ~1x per second to avoid spam

#### 3. WiFi Manager
**No changes needed** - let it run as-is, no WiFi events will come, web server will just work without network

## Technical Implementation Plan

### 1. Kconfig Configuration
Create build-time switches in `main/Kconfig.projbuild`:
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

### 2. Implementation Strategy - Separate Source Files
**Elegant approach: Same headers, different source files per build variant**

No `#ifdef` cluttering the code! CMake selects the appropriate source file:

**Distance Sensor:**
- `distance_sensor.h` - Same API for both variants
- `distance_sensor.c` - Real hardware implementation
- `distance_sensor_sim.c` - Simulator implementation

**LED Controller:**  
- `led_controller.h` - Same API for both variants
- `led_controller.c` - Real RMT hardware implementation
- `led_controller_sim.c` - Simulator implementation

**Task:** Implement complete APIs in `_sim.c` files - internal implementation doesn't matter, only API compatibility.

### 3. Simulator Implementations

#### 3.1 Distance Sensor Simulator (`distance_sensor_sim.c`):
```c
// Simulated sensor with animated distance sweep
static void distance_sensor_task(void* pvParameters) {
    static uint16_t sim_distance = 50;  // Start at 5cm
    static int8_t direction = 1;        // 1 = increasing, -1 = decreasing
    
    while(1) {
        // Animate distance: 5cm → 60cm → 5cm (1mm steps)
        sim_distance += direction;
        if (sim_distance >= 600) direction = -1;  // 60cm
        if (sim_distance <= 50)  direction = 1;   // 5cm
        
        distance_measurement_t sim_data = {
            .distance_mm = sim_distance,
            .timestamp_us = esp_timer_get_time(),
            .status = DISTANCE_SENSOR_OK
        };
        
        xQueueSend(processed_measurement_queue, &sim_data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 second steps
    }
}
```

#### 3.2 LED Controller Simulator (`led_controller_sim.c`):

**LED Visualization Strategy:**
```c
// Rate-limited output - only display ~1x per second
static uint64_t last_display_time = 0;

esp_err_t led_show(void) {
    uint64_t now = esp_timer_get_time();
    if (now - last_display_time < 1000000) {  // 1 second = 1,000,000 us
        return ESP_OK;  // Suppress output, just return success
    }
    last_display_time = now;
    
    // Now do the emoji output
    printf("\n[LED Strip]: ");
    for (int i = 0; i < led_count; i++) {
        led_color_t* color = &led_buffer[i];
        
        if (color->red > 200 && color->green < 50 && color->blue < 50) {
            printf("🔴");  // Red
        } else if (color->green > 200 && color->red < 50 && color->blue < 50) {
            printf("🟢");  // Green  
        } else if (color->blue > 200 && color->red < 50 && color->green < 50) {
            printf("🔵");  // Blue
        } else if (color->red > 200 && color->blue > 200 && color->green < 50) {
            printf("🟣");  // Magenta/Purple
        } else if (color->red > 200 && color->green > 200 && color->blue < 50) {
            printf("🟡");  // Yellow
        } else if (color->red + color->green + color->blue > 600) {
            printf("⚪");  // White/bright
        } else if (color->red + color->green + color->blue > 100) {
            printf("⚫");  // Dim/mixed
        } else {
            printf("⚫");  // Off
        }
    }
    printf("\n");
    return ESP_OK;
}
```
```

### 4. Build Configuration - CMake Source Selection
```cmake
# In components/distance_sensor/CMakeLists.txt
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "distance_sensor_sim.c")
else()
    set(COMPONENT_SRCS "distance_sensor.c")
endif()

# In components/led_controller/CMakeLists.txt  
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "led_controller_sim.c")
else()
    set(COMPONENT_SRCS "led_controller.c")
endif()
```

## Success Criteria
- [ ] Both hardware and emulator builds compile successfully  
- [ ] Hardware build maintains full functionality (no regressions)
- [ ] Emulator build runs in QEMU without hardware dependencies
- [ ] Distance sensor shows animated 5→60→5cm pattern in QEMU console
- [ ] LED strip visualization updates in terminal showing distance changes  
- [ ] Web interface loads (even without WiFi network)
- [ ] Clean CMake-based build variant selection

## Implementation Instructions
1. **Start with Kconfig setup** - enable emulator builds
2. **Implement distance_sensor_sim.c** - complete API with animation  
3. **Implement led_controller_sim.c** - complete API with rate-limited emoji output
4. **Update CMakeLists.txt** files for conditional compilation
5. **Test in QEMU** - verify LED animation reflects distance changes
6. **Verify hardware build** still works unchanged

**KISS Principle: No web interfaces, no complex patterns, no network mocking - just working emulator build with visual feedback**