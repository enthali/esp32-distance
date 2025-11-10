Debugging Guide
===============

The ESP32 Template supports full GDB debugging in both emulator (QEMU) and hardware modes. This guide focuses on QEMU debugging since it's fully supported in GitHub Codespaces.

Quick Start: Debugging in QEMU
-------------------------------

1. Start QEMU Debug Server
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The QEMU emulator automatically starts with GDB support enabled:

.. code-block:: bash

   ./tools/run-qemu-network.sh

This starts QEMU in debug mode, waiting for a debugger connection on port 3333.

2. Start Debugging in VS Code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Simply press **F5** or use the Debug panel:

1. Open the Debug view (Ctrl+Shift+D / Cmd+Shift+D)
2. Select "**QEMU: GDB**" from the dropdown
3. Click the green play button or press F5

VS Code will:

- ✅ Connect to QEMU's GDB server (port 3333)
- ✅ Load symbols from the built ELF file
- ✅ Break at ``app_main()``
- ✅ Show full call stack and variables

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

QEMU Debug Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~

The ``.vscode/launch.json`` contains the QEMU debug configuration:

.. code-block:: json

   {
       "name": "QEMU: GDB",
       "type": "cppdbg",
       "request": "launch",
       "program": "${workspaceFolder}/build/esp32-template.elf",
       "cwd": "${workspaceFolder}",
       "MIMode": "gdb",
       "miDebuggerPath": "xtensa-esp32-elf-gdb",
       "miDebuggerServerAddress": "localhost:3333"
   }

**Key Settings:**

- **program**: Path to ELF file with debug symbols
- **miDebuggerPath**: GDB executable (ESP32 toolchain)
- **miDebuggerServerAddress**: QEMU GDB server (port 3333)

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
