Debugging Guide
===============

The ESP32 Distance Sensor project supports full GDB debugging in both emulator (QEMU) and hardware modes. This guide focuses on QEMU debugging since it's fully supported in GitHub Codespaces.

Quick Start: Debugging in QEMU
-------------------------------

**It's as simple as pressing F5!**

1. Open your source file in VS Code (e.g., ``main/main.c``)
2. Set breakpoints by clicking in the gutter (left of line numbers)
3. Press **F5** (or click the green play button in Debug panel)

That's it! VS Code automatically:

- ✅ Builds your project if needed
- ✅ Starts QEMU in debug mode
- ✅ Connects GDB debugger
- ✅ Breaks at your breakpoints
- ✅ Stops QEMU when you stop debugging

.. tip::
   **No manual setup needed!** The ``preLaunchTask`` in ``.vscode/launch.json`` automatically runs QEMU debug server before attaching the debugger.

Debugging Features
------------------

Breakpoints
~~~~~~~~~~~

Set breakpoints by clicking in the editor gutter (left of line numbers):

.. code-block:: c

   void app_main(void)
   {
       // Breakpoint here: Click in gutter at line number
       ESP_LOGI(TAG, "ESP32 Application Starting...");
       
       // Conditional breakpoint: Right-click → Add Conditional Breakpoint
       if (value < 10) {
           // Break only when value < 10
       }
   }

**Breakpoint Types:**

- **Line Breakpoint**: Break when reaching specific line
- **Conditional Breakpoint**: Break only when condition is true
- **Logpoint**: Log message without stopping (useful for tracing)

Step Through Code
~~~~~~~~~~~~~~~~~

Use VS Code debug toolbar or keyboard shortcuts:

- **F10** - Step Over (execute current line, don't enter functions)
- **F11** - Step Into (enter function calls)
- **Shift+F11** - Step Out (return from current function)
- **F5** - Continue (run until next breakpoint)

Inspect Variables
~~~~~~~~~~~~~~~~~

**Variables Panel**: Shows local variables and function parameters automatically

**Watch Panel**: Add custom expressions to monitor

.. code-block:: c

   // Example: Monitor a struct member
   config.led_count
   
   // Example: Monitor array element
   buffer[5]
   
   // Example: Evaluate expression
   (value * 2) > threshold

Call Stack
~~~~~~~~~~

The Call Stack panel shows the complete function call hierarchy:

.. code-block:: text

   #0  my_function() at main.c:42
   #1  process_data() at main.c:128
   #2  app_main() at main.c:200
   #3  main_task() at cpu_start.c:...

Click any frame to inspect its local variables and source code.

Debug Console
~~~~~~~~~~~~~

Execute GDB commands directly in the Debug Console:

.. code-block:: text

   # Print variable
   p my_variable
   
   # Print in hex
   p/x my_variable
   
   # Print array
   p my_array[0]@10
   
   # Call function
   call my_debug_function()

Hardware Debugging
------------------

Debugging on Real ESP32
~~~~~~~~~~~~~~~~~~~~~~~

For hardware debugging, you need a JTAG adapter. The template doesn't include hardware debugging configuration by default (focus is on QEMU).

For production projects, see `ESP-IDF JTAG Debugging <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/jtag-debugging/>`_ documentation.

Debug Configuration
-------------------

How It Works
~~~~~~~~~~~~

The ``.vscode/launch.json`` configuration handles everything automatically:

.. code-block:: json

   {
       "name": "QEMU ESP32 Debug (with Graphics)",
       "preLaunchTask": "Run QEMU Debug",      // Starts QEMU before debugging
       "postDebugTask": "Stop QEMU",            // Stops QEMU when you stop debugging
       "miDebuggerServerAddress": "localhost:3333"  // GDB connects here
   }

**What happens when you press F5:**

1. ``preLaunchTask`` runs: QEMU starts in debug mode (waits for GDB)
2. GDB connects to QEMU on port 3333
3. Your breakpoints are set
4. Program runs until it hits a breakpoint
5. When you stop debugging, ``postDebugTask`` stops QEMU cleanly

.. note::
   You don't need to manually start QEMU! The VS Code tasks handle it automatically.

Customization
~~~~~~~~~~~~~

If you need to modify the ELF file path (e.g., after renaming your project):

.. code-block:: json

   "program": "${workspaceFolder}/build/your-project-name.elf"

Advanced Debugging
------------------

Debugging FreeRTOS Tasks
~~~~~~~~~~~~~~~~~~~~~~~~

View FreeRTOS task information:

.. code-block:: text

   # In Debug Console
   info threads
   
   # Switch to specific task
   thread 3

Memory Inspection
~~~~~~~~~~~~~~~~~

Inspect memory directly:

.. code-block:: text

   # View memory at address (hex)
   x/16x 0x3FFB0000
   
   # View memory as string
   x/s 0x3FFB0000
   
   # Examine stack
   x/32x $sp

Performance Analysis
~~~~~~~~~~~~~~~~~~~~

Measure execution time:

.. code-block:: c

   // Add timing code
   uint64_t start = esp_timer_get_time();
   
   // Your code here
   my_function();
   
   uint64_t elapsed = esp_timer_get_time() - start;
   ESP_LOGI(TAG, "Execution time: %llu us", elapsed);

Set breakpoints before and after, inspect ``elapsed`` value.

Common Debugging Scenarios
---------------------------

Crash Analysis
~~~~~~~~~~~~~~

When ESP32 crashes, look for:

1. **Stack trace** in serial output
2. **Program Counter (PC)** - use ``addr2line`` to find source line
3. **Register dump** - shows CPU state at crash

.. code-block:: bash

   # Convert crash address to source line
   xtensa-esp32-elf-addr2line -e build/esp32-template.elf 0x400d1234

Memory Leaks
~~~~~~~~~~~~

Monitor heap usage:

.. code-block:: c

   ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
   ESP_LOGI(TAG, "Min free heap: %d bytes", esp_get_minimum_free_heap_size());

Stack Overflow
~~~~~~~~~~~~~~

Check stack high water mark:

.. code-block:: c

   UBaseType_t stack_left = uxTaskGetStackHighWaterMark(NULL);
   ESP_LOGI(TAG, "Stack space left: %d words", stack_left);

Troubleshooting
---------------

GDB Won't Connect
~~~~~~~~~~~~~~~~~

**Problem:** VS Code can't connect to QEMU

**Solutions:**

1. Ensure QEMU is running: ``ps aux | grep qemu``
2. Check port 3333 is available: ``lsof -i :3333``
3. Restart QEMU: ``./tools/stop_qemu.sh && ./tools/run-qemu-network.sh``

No Debug Symbols
~~~~~~~~~~~~~~~~

**Problem:** Can't see source code or variable names

**Solutions:**

1. Ensure debug build: ``idf.py menuconfig`` → Component config → Compiler options → Optimization Level → Debug (-Og)
2. Rebuild: ``idf.py fullclean build``

Breakpoint Not Hit
~~~~~~~~~~~~~~~~~~

**Problem:** Breakpoint is set but never triggers

**Checks:**

1. Code is actually executed (not in dead code path)
2. Compiler didn't optimize code away (check disassembly)
3. Breakpoint is in correct file (check paths match)

Tips and Best Practices
------------------------

Efficient Debugging
~~~~~~~~~~~~~~~~~~~

✅ **Use Logpoints**: Don't stop execution, just log information

✅ **Conditional Breakpoints**: Break only when specific conditions occur

✅ **Watch Expressions**: Monitor key variables without manual inspection

✅ **Call Stack Navigation**: Quickly find where problems originate

Logging vs Debugging
~~~~~~~~~~~~~~~~~~~~~

**Use Logging For:**

- Production code monitoring
- Long-running operations
- Intermittent issues
- Field diagnostics

**Use Debugging For:**

- Development and troubleshooting
- Complex state inspection
- Step-by-step execution analysis
- One-time investigation

Debug Optimization
~~~~~~~~~~~~~~~~~~

**During Development:**

- Use Debug (-Og) optimization for best debugging experience
- Enable all debug symbols

**For Release:**

- Switch to Release optimization (-O2 or -Os)
- Disable verbose logging
- Keep ESP_LOGI for important events

Resources
---------

- `ESP-IDF Debugging Guide <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/jtag-debugging/>`_
- `GDB Manual <https://sourceware.org/gdb/current/onlinedocs/gdb/>`_
- `VS Code Debugging <https://code.visualstudio.com/docs/editor/debugging>`_

Next Steps
----------

- :doc:`qemu-emulator` - Learn more about QEMU emulation
- :doc:`qemu-network-internals` - Understand network implementation
- :doc:`devcontainer` - Set up your development environment
