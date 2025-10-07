# Startup Test Design

## Design Traceability

| Design ID | Implements Requirement | Priority |
|-----------|------------------------|----------|
| DSN-STARTUP-1 | REQ-STARTUP-1 | Mandatory |
| DSN-STARTUP-2 | REQ-STARTUP-2 | Mandatory |
| DSN-STARTUP-3 | REQ-STARTUP-3 | Important |

## Target Design Architecture

### DSN-STARTUP-1: LED Controller Dependency Design

Addresses: REQ-STARTUP-1

Design: Startup test executes after LED controller initialization in main() function.

- **Execution Point**: Called in main.c after `led_controller_init()` succeeds
- **Dependency Check**: Uses `led_is_initialized()` to verify LED controller readiness
- **Error Handling**: Returns `ESP_ERR_INVALID_STATE` if LED controller not initialized
- **Integration**: Direct function call from main(), no separate task required

Architecture Flow:

```c
main() → led_controller_init() → led_running_test_single_cycle() → normal_operation()
```

Validation: LED controller must be successfully initialized before startup test can execute.

### DSN-STARTUP-2: Sequential LED Pattern Algorithm Design

Addresses: REQ-STARTUP-2

Design: Single moving LED pattern implemented using sequential clear-set-show operations.

**Core Algorithm** (`led_running_test_single_cycle`):

```c
for (uint16_t i = 0; i < led_count; i++) {
    // Clear previous LED (wrap around for first LED)
    if (i > 0) {
        led_clear_pixel(i - 1);
    } else {
        led_clear_pixel(led_count - 1);  // Clear last LED on first iteration
    }
    
    // Set current LED
    led_set_pixel(i, color);
    led_show();                    // Update physical LEDs
    
    vTaskDelay(pdMS_TO_TICKS(delay_ms));  // Timing control
}
```

**Design Characteristics**:

- **Single LED Active**: Only one LED illuminated at any time (clear previous, set current)
- **Sequential Progression**: LEDs activated from index 0 to (led_count-1)
- **Configurable Color**: Color parameter allows different startup patterns
- **Configurable Timing**: Delay parameter controls speed (default 50ms per LED)
- **Immediate Updates**: `led_show()` called after each LED change for real-time feedback

**Current Implementation**:

- Called as: `led_running_test_single_cycle(LED_COLOR_GREEN, 50)`
- Green color for startup indication
- 50ms delay between LED activations
- Single cycle (not repeating)

Validation: LEDs light sequentially from first to last position with visible timing.

### DSN-STARTUP-3: Startup Integration and Cleanup Design

Addresses: REQ-STARTUP-3

Design: One-time execution pattern with proper LED state management.

**Integration Pattern** (from main.c):

```c
// 1. Initialize LED controller
led_controller_init(&led_config);

// 2. Clear initial state
led_clear_all();
led_show();

// 3. Execute startup test
ESP_LOGI(TAG, "Running one-time LED hardware test...");
led_running_test_single_cycle(LED_COLOR_GREEN, 50);
ESP_LOGI(TAG, "Hardware test completed");

// 4. Clean up for normal operation
led_clear_all();
led_show();

// 5. Continue with normal initialization
```

**Design Characteristics**:

- **One-Time Execution**: Called once during startup, not as background task
- **State Cleanup**: LEDs cleared before and after test
- **Logging Integration**: ESP_LOG* statements for startup visibility
- **Non-Blocking**: Executes synchronously in main thread
- **Clean Transition**: All LEDs off after test, ready for normal operation

**Performance**:

- Total execution time for 40 LEDs: ~2 seconds (40 × 50ms)
- Memory usage: No additional allocation beyond LED controller
- CPU impact: Minimal, just timing delays

Validation: Test completes within reasonable time, all LEDs cleared after completion.

## Implementation Architecture

### Component Structure

```text
main/components/startup_tests/
├── led_running_test.h          // Public API
├── led_running_test.c          // Implementation  
└── CMakeLists.txt              // Build config
```

### API Design

- **Primary Function**: `led_running_test_single_cycle(color, delay_ms)`
- **Extended Functions**: `led_running_test_multiple_cycles()`, `led_running_test_rainbow()`
- **Error Handling**: ESP-IDF error codes (`ESP_OK`, `ESP_ERR_INVALID_STATE`)
- **Dependencies**: LED controller component

### Alternative Patterns Available

The implementation provides additional patterns for extended testing:

- **Multiple Cycles**: `led_running_test_multiple_cycles()` for repeated patterns
- **Rainbow Effect**: `led_running_test_rainbow()` for color-changing sequences

These are available for advanced startup testing or diagnostic modes but not used in basic startup sequence.
