LED Controller Design Specification
=====================================

This document specifies the design of the WS2812 LED controller component, using ESP32 RMT 
peripheral for precise timing and providing a simple pixel-level control API.

**Design Version**: 1.0  
**Last Updated**: 2025-11-10

.. contents:: Table of Contents
   :local:
   :depth: 2

Overview
--------

The LED controller abstracts WS2812 timing complexity using ESP32 RMT peripheral, providing 
a simple buffer-based API with batch operations and color utilities.

Hardware Abstraction
--------------------

.. spec:: RMT Peripheral Hardware Abstraction
   :id: SPEC_LED_ARCH_1
   :links: REQ_LED_1
   :status: approved
   :tags: led, rmt, hardware

   **Design:** ESP32 RMT (Remote Control) peripheral abstraction for WS2812 timing generation.

   **Implementation:**

   - RMT Channel Configuration: 80MHz resolution, configurable GPIO pin, 64-symbol memory blocks
   - Encoder Configuration: Bytes encoder with precise WS2812 timing (T0H=0.4µs, T0L=0.8µs, T1H=0.8µs, T1L=0.4µs)
   - Transmission Queue: 4-deep queue for overlapping operations
   - Clock Source: Default ESP32 RMT clock for consistent timing
   - Channel Management: Single channel allocation with proper cleanup

   **Validation:** RMT channel created successfully, timing parameters match WS2812 datasheet, 
   transmission completes without errors.

.. spec:: RAM Buffer Architecture
   :id: SPEC_LED_ARCH_2
   :links: REQ_LED_2, REQ_LED_3
   :status: approved
   :tags: led, memory, buffer

   **Design:** In-memory LED state buffer for performance optimization and atomic updates.

   **Implementation:**

   - Buffer Structure: Array of ``led_color_t`` structures (3 bytes per LED: R, G, B)
   - Dynamic Allocation: ``malloc()`` during initialization based on configured LED count (REQ_LED_3)
   - State Separation: RAM buffer independent from physical LED state until ``led_show()``
   - Update Pattern: Modify buffer → call ``led_show()`` → physical update
   - Memory Management: Allocation during init, deallocation during cleanup

   **Validation:** Buffer allocates correctly for configured LED count, updates modify only 
   buffer until show, memory freed on cleanup.

API Specifications
------------------

.. spec:: Pixel-Level Control API
   :id: SPEC_LED_API_1
   :links: REQ_LED_2
   :status: approved
   :tags: led, api, control

   **Design:** Individual LED pixel manipulation with bounds checking and color utilities.

   **Implementation:**

   - ``led_set_pixel(index, color)``: Set specific LED color with index validation
   - ``led_get_pixel(index)``: Read current LED color from buffer
   - ``led_clear_pixel(index)``: Turn off specific LED (set to black)
   - Color Structure: ``led_color_t`` with 8-bit RGB components
   - Predefined Colors: Constants for common colors (RED, GREEN, BLUE, WHITE, OFF, etc.)
   - Bounds Checking: Index validation against configured LED count

   **Validation:** Index validation prevents buffer overruns, color values stored accurately, 
   predefined colors work correctly.

.. spec:: Batch Operations API
   :id: SPEC_LED_API_2
   :links: REQ_LED_2, REQ_LED_4
   :status: approved
   :tags: led, api, batch

   **Design:** Efficient batch operations for common patterns and hardware updates.

   **Implementation:**

   - ``led_clear_all()``: Set all LEDs to off state in single operation
   - ``led_show()``: Transmit complete buffer to hardware via RMT (enables REQ_LED_4)
   - Color Utilities: ``led_color_rgb()`` constructor, ``led_color_brightness()`` scaling
   - Status Functions: ``led_get_count()``, ``led_is_initialized()`` for state queries
   - Atomic Updates: Buffer modifications independent until ``led_show()`` called

   **Validation:** Clear all zeros entire buffer, show triggers RMT transmission, utilities 
   produce correct colors.

Timing and Data Format
-----------------------

.. spec:: WS2812 Timing Specification
   :id: SPEC_LED_TIMING_1
   :links: REQ_LED_1
   :status: approved
   :tags: led, timing, ws2812

   **Design:** Precise WS2812 protocol timing using RMT encoder configuration.

   **Implementation:**

   - Bit 0 Encoding: High 0.4µs (32 ticks), Low 0.8µs (64 ticks)
   - Bit 1 Encoding: High 0.8µs (64 ticks), Low 0.4µs (32 ticks)
   - Reset Period: 50µs (4000 ticks) low signal between frames
   - Clock Resolution: 80MHz RMT clock for 12.5ns tick precision
   - MSB First: Most significant bit transmitted first per WS2812 protocol

   **Validation:** Timing measurements match WS2812 datasheet specifications, LED strips 
   respond correctly.

.. spec:: Color Representation and Conversion
   :id: SPEC_LED_DATA_1
   :links: REQ_LED_4
   :status: approved
   :tags: led, color, conversion

   **Design:** RGB color representation with GRB hardware conversion for WS2812 compatibility.

   **Implementation:**

   - API Color Format: RGB (Red, Green, Blue) for user-friendly interface
   - Hardware Format: GRB (Green, Red, Blue) as required by WS2812 LEDs
   - Conversion Logic: Reorder RGB→GRB during ``led_show()`` transmission preparation
   - Data Buffer: Temporary allocation for GRB transmission data
   - Brightness Scaling: Integer arithmetic for brightness adjustment without floating point

   **Validation:** Color values convert correctly RGB→GRB, brightness scaling maintains color 
   ratios, no precision loss.

Memory and Error Handling
--------------------------

.. spec:: Dynamic Memory Management
   :id: SPEC_LED_MEM_1
   :links: REQ_LED_3
   :status: approved
   :tags: led, memory, resource

   **Design:** Controlled dynamic allocation with proper cleanup and error handling.

   **Implementation:**

   - Initialization Allocation: LED buffer ``malloc()`` based on configured count
   - Validation: LED count bounds (1-1000) to prevent excessive allocation
   - Cleanup: ``free()`` buffer and reset pointers during deinitialization
   - Temporary Allocation: GRB data buffer during transmission (freed immediately)
   - Error Recovery: Cleanup partial initialization on failure

   **Validation:** Memory allocated correctly, no leaks after deinitialization, error paths 
   clean up properly.

.. spec:: Error Handling and Validation
   :id: SPEC_LED_ERR_1
   :links: REQ_LED_2, REQ_LED_3
   :status: approved
   :tags: led, error-handling, validation

   **Design:** Comprehensive input validation and state checking with appropriate error codes.

   **Implementation:**

   - State Validation: Check initialization before operations
   - Bounds Checking: LED index validation against configured count (REQ_LED_3)
   - Parameter Validation: Non-null config, valid LED count range
   - RMT Error Handling: Propagate RMT peripheral errors to caller
   - Graceful Degradation: Safe operation when not initialized (return errors, not crash)

   **Validation:** Invalid inputs return appropriate error codes, operations fail safely, 
   system remains stable.

Simulator Specification
-----------------------

.. spec:: LED Controller Simulator
   :id: SPEC_LED_SIM_1
   :links: REQ_SYS_SIM_1, SPEC_LED_ARCH_2
   :status: approved
   :tags: led, simulator, qemu

   **Design:** Provide a simulator implementation for the LED controller that exposes the full 
   public API (``led_controller.h``) while replacing RMT transmission with a rate-limited 
   terminal visualization.

   **Implementation:**

   - API Compatibility: The simulator SHALL implement all public functions declared in 
     ``led_controller.h`` and return the same error codes and behavior as the hardware 
     implementation
   - Buffer Semantics: Maintain the same in-memory buffer and batch ``led_show()`` semantics 
     as the hardware implementation
   - Visualization: Rate-limited (≈1Hz) terminal output using Unicode emoji blocks 
     (🔴🟢🔵🟡🟣⚪⚫) to represent per-pixel color state
   - Non-intrusive: The simulator SHOULD NOT change header files, configuration, or 
     higher-layer logic (e.g., display logic)

   **Validation:** Simulator build compiles with ``CONFIG_TARGET_EMULATOR=y``, ``led_show()`` 
   logs appear ~1x/sec and reflect buffer state.
