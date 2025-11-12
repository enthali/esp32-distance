Workshop Exercises
==================

These hands-on exercises help you master ESP32 development using GitHub Copilot and the GitHub Coding Agent. Work through them at your own pace and experiment with your own ideas!

.. note::

   **GitHub Copilot Required:** These exercises utilize GitHub Copilot's agent capabilities. 
   If you don't have access to the GitHub Coding Agent, you can:
   
   * Use **GitHub Copilot Chat** in Agent Mode
   * Complete exercises manually following the coding standards
   * Request a free trial at `GitHub Copilot <https://github.com/features/copilot>`_

Prerequisites
-------------

Before starting exercises, ensure you have:

✓ **Hardware assembled** - See :doc:`hardware-setup` for wiring instructions
✓ **Firmware flashed** - Follow :doc:`getting-started` to build and flash
✓ **System working** - LEDs respond to distance sensor
✓ **Codespace running** - Development environment active
✓ **GitHub Copilot enabled** - Access to Coding Agent or Chat

.. tip::

   **QEMU Testing:** You can complete most exercises using QEMU emulation if you 
   don't have hardware available. Some exercises work better with real hardware.

Exercise A: Extend LED Patterns
--------------------------------

**Difficulty:** Intermediate  
**Time:** 30-45 minutes  
**Skills:** Requirements engineering, design documentation, implementation

Objective
~~~~~~~~~

Learn how to use GitHub Coding Agent to implement a feature request with full requirements 
and design documentation traceability.

**Current Behavior:**

* When distance < minimum threshold → First LED turns red
* Static red indicator, no animation

**Desired Behavior:**

* When distance < minimum threshold → Every 10th LED flashes red
* Flashing frequency: 1 second (0.5s on, 0.5s off)
* No interference with distance measurement timing

Step 1: Create GitHub Issue
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open GitHub Copilot Chat in your Codespace and enter this prompt:

.. code-block:: text

   I want you to create an issue on GitHub that we will then assign to the 
   GitHub coding agent: We want to change the behavior of the system. 
   Right now, when the measurement is below the minimum threshold, the 
   first LED is red. However, I want every 10th LED to flash red with a 
   1-second frequency. Please update the requirements document 
   (docs/11_requirements/req_display.rst), update and extend the design 
   document (docs/12_design/spec_display.rst), and implement the solution.

This guides Copilot Chat Agent to:

* Generate a GitHub issue via GitHub MCP server
* Reference requirements and design documentation
* Request full documentation + implementation updates

Step 2: Assign to Coding Agent
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You have two options:

**Option A: Assign via GitHub Website**

1. Navigate to repository's **"Issues"** tab on GitHub
2. Locate the newly created issue
3. Click **"Assignees"** → Select **GitHub Coding Agent**

**Option B: Assign via Copilot Chat**

Enter this prompt in Copilot Chat:

.. code-block:: text

   Please assign the new issue to the GitHub coding agent.

Either method routes the issue to the Coding Agent for implementation.

Step 3: Watch the PR Process
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The GitHub Coding Agent will automatically begin work by creating a pull request (PR).

**Monitor Progress:**

1. **Navigate to Pull Requests tab** in your repository
2. **Locate the new PR** - Title references the issue
3. **Follow the timeline:**
   
   * Agent sets up development environment (~2 minutes)
   * Updates requirements document (``req_display.rst``)
   * Updates design document (``spec_display.rst``)
   * Implements code changes in ``display_logic.c``
   * Commits with proper traceability references
   * Runs automated tests/checks
   * Requests your review

.. note::

   **Typical Duration:** 10-15 minutes for complete PR creation

**PR Monitoring Tips:**

* View **"Commits"** tab to see incremental changes
* Check **"Files changed"** to review modifications
* Watch **"Checks"** for CI/CD pipeline status
* Read **"Conversation"** for agent's explanations

Step 4: Review and Test
~~~~~~~~~~~~~~~~~~~~~~~~

**In VS Code:**

1. **Check out the PR branch:**

   .. code-block:: bash

      git fetch origin
      git checkout fix/led-flashing-pattern  # Branch name from PR

2. **Build the project:**

   .. code-block:: bash

      idf.py build

3. **Flash to hardware or run in QEMU:**

   .. code-block:: bash

      # Hardware
      idf.py flash monitor
      
      # QEMU
      ./tools/qemu/run_qemu.sh

4. **Test the new behavior:**
   
   * Move object **below minimum threshold** (< 10cm)
   * Watch every 10th LED flash red (1 Hz)
   * Verify no impact on distance measurement responsiveness

**Review Documentation:**

1. Open ``docs/11_requirements/req_display.rst``
2. Look for **new requirement** added by Coding Agent
3. Check **requirement ID** and **traceability links**
4. Open ``docs/12_design/spec_display.rst``
5. Review **design specification** for flashing algorithm
6. Verify **bidirectional traceability** (requirements ↔ design ↔ code)

**Check Code Quality:**

The Coding Agent follows project coding standards:

* ✅ Requirements traceability comments in code
* ✅ Design rationale in function headers
* ✅ ESP-IDF naming conventions
* ✅ Proper error handling
* ✅ Non-blocking timer implementation

.. tip::

   **Key Learning:** Notice how the Coding Agent implemented time-based flashing 
   without using ``vTaskDelay()`` which would block the measurement task! 
   This demonstrates proper FreeRTOS task design.

Step 5: Approve and Merge
~~~~~~~~~~~~~~~~~~~~~~~~~~

If everything works:

1. **Add review comment** in GitHub PR
2. **Approve the PR**
3. **Merge to main branch**

If issues found:

1. **Comment on specific lines** in PR
2. **Request changes** from Coding Agent
3. **Agent will update** and push new commits

Creative LED Pattern Ideas
~~~~~~~~~~~~~~~~~~~~~~~~~~

Want to go beyond the exercise? Try these extensions:

**Distance-Based Color Gradients:**

* Green for close range (10-20cm)
* Yellow for mid-range (20-35cm)
* Red for far range (35-50cm)
* Smooth color transitions

**Animated Effects:**

* LED "wave" effect that sweeps along strip
* Breathing animation (fade in/out)
* Rainbow color cycling
* Proximity-based speed changes

**Threshold Alerts:**

* Rapid flashing when object too close (< 5cm)
* Slow pulse when object too far (> 50cm)
* All LEDs flash on error condition

**Custom Patterns:**

* Progress bar showing distance as percentage
* Alternating colors based on distance zones
* Binary counter display (nerdy!)
* Your own creative ideas!

.. tip::

   **Implementation Approach:** For each new idea, follow the same workflow:
   
   1. Create GitHub issue describing desired behavior
   2. Assign to Coding Agent
   3. Review PR with documentation updates
   4. Test and iterate

Exercise B: Improve Distance Filtering
---------------------------------------

**Difficulty:** Advanced  
**Time:** 45-60 minutes  
**Skills:** Signal processing, numerical methods, cross-component changes

Objective
~~~~~~~~~

Learn how GitHub Coding Agent handles **multi-file refactoring** across components.

Background
~~~~~~~~~~

This project previously used floating-point arithmetic for distance calculations. 
With a single GitHub issue, the Coding Agent converted the entire system to 
**fixed-point integer math**, improving:

* Performance (no FPU overhead)
* Predictability (deterministic timing)
* Memory usage (smaller binary)

.. note::

   **Historical Context:** The codebase was migrated from ``float`` to ``int`` 
   using fixed-point arithmetic (16-bit fractional representation). You can reverse 
   this or explore other numerical methods!

Your Mission
~~~~~~~~~~~~

Choose one of these improvements:

**Option 1: Revert to Floating Point**

Create an issue requesting conversion back to floating-point arithmetic:

.. code-block:: text

   I want to use floating-point arithmetic for distance calculations instead 
   of fixed-point integers. Please update all distance sensor and display logic 
   components to use `float` types, update calculations accordingly, and verify 
   accuracy is maintained. Update requirements and design docs.

**Option 2: Implement Kalman Filtering**

Add advanced filtering to smooth noisy sensor readings:

.. code-block:: text

   The distance sensor readings are noisy. Please implement a simple Kalman 
   filter to smooth the measurements. Add a new requirement for filtering, 
   update the design specification with filter equations, and implement in 
   the distance_sensor component. Ensure filter state persists across measurements.

**Option 3: Add Moving Average Filter**

Implement a simple but effective noise reduction:

.. code-block:: text

   Add a 5-sample moving average filter to the distance sensor readings to 
   reduce noise. Document as a new requirement, specify filter design, and 
   implement a circular buffer for efficient computation. Verify filtering 
   does not introduce unacceptable latency.

What to Observe
~~~~~~~~~~~~~~~

Watch how the Coding Agent:

* **Identifies affected files** across multiple components
* **Updates multiple requirements** (sensor + display)
* **Maintains traceability** across all changes
* **Preserves interfaces** between components
* **Tests thoroughly** to prevent regressions

.. tip::

   **Multi-Component Changes:** This exercise demonstrates the Coding Agent's 
   ability to understand system architecture and make coordinated changes 
   across loosely-coupled components.

Exercise C: LED Animation on Website
-------------------------------------

**Difficulty:** Advanced  
**Time:** 60-90 minutes  
**Skills:** Web development, real-time updates, full-stack integration

Objective
~~~~~~~~~

Add real-time LED strip visualization to the web interface showing current LED states.

Challenge Description
~~~~~~~~~~~~~~~~~~~~~

**Current Web Interface:**

* Static HTML page
* Displays distance reading (when implemented)
* No LED visualization

**Desired Web Interface:**

* Real-time LED strip animation
* Mirrors physical LED states
* Updates 10-30 times per second
* Shows colors, brightness, animations

Implementation Approaches
~~~~~~~~~~~~~~~~~~~~~~~~~

**Approach 1: WebSocket Streaming**

Create an issue requesting WebSocket-based real-time updates:

.. code-block:: text

   I want to see the LED strip state in real-time on the web interface. 
   Please add WebSocket support to the web server, stream LED state updates 
   from the display_logic component, and create an HTML5 canvas visualization 
   that shows all 40 LEDs with their current colors. Update requirements for 
   real-time web interface.

**Approach 2: SSE (Server-Sent Events)**

Simpler alternative using HTTP streaming:

.. code-block:: text

   Add real-time LED visualization using Server-Sent Events (SSE). Stream 
   LED state updates to the web interface and create a simple HTML/CSS 
   representation of the LED strip. Include CSS animations for smooth 
   color transitions.

**Approach 3: AJAX Polling**

Simplest implementation (less smooth but easier):

.. code-block:: text

   Add a REST API endpoint that returns current LED strip state as JSON. 
   Create a JavaScript polling mechanism (10 Hz) that fetches LED states 
   and updates an HTML canvas or CSS grid visualization. Include color 
   interpolation for smooth appearance.

Bonus Challenges
~~~~~~~~~~~~~~~~

Once basic visualization works, try these enhancements:

* **3D LED strip visualization** using Three.js or WebGL
* **Color picker interface** to manually set LED colors
* **Animation presets** (rainbow, wave, pulse) selectable from web UI
* **Record and replay** LED patterns
* **Multi-device sync** showing same animation across multiple ESP32s

.. note::

   **Competition:** First person to implement this wins bragging rights! 🏆
   
   Share your solution in GitHub Discussions!

What You'll Learn
~~~~~~~~~~~~~~~~~

This exercise covers:

* Real-time web communication protocols
* Efficient data serialization (LED states)
* Browser rendering optimization
* ESP32 memory management for web sockets
* HTML5 Canvas or CSS Grid layouts
* JavaScript animation techniques

.. tip::

   **Memory Constraint:** ESP32 has limited RAM! Consider sending only 
   changed LEDs (delta encoding) instead of full strip state every frame.

Exercise D: Custom Feature (Your Choice)
-----------------------------------------

**Difficulty:** Your choice  
**Time:** Varies  
**Skills:** Creativity, problem-solving, system integration

Your Mission
~~~~~~~~~~~~

Design and implement your own feature using GitHub Coding Agent!

Feature Ideas:

**Hardware Enhancements:**

* Add buzzer for audio alerts at thresholds
* Integrate temperature sensor (BME280)
* Add push button for mode switching
* Connect OLED display (SSD1306) for readings

**Software Features:**

* Data logging to SD card or NVS
* MQTT integration for IoT dashboard
* Home Assistant integration
* Bluetooth LE proximity alerts
* OTA (Over-The-Air) firmware updates
* **Dual-layer LED display with zone-based guidance** (see example below)

**User Interface:**

* Configuration web page (see existing design!)
* Mobile app using BLE
* REST API for automation
* Captive portal improvements

**Algorithm Enhancements:**

* Multi-zone detection (multiple sensors)
* Object speed calculation (velocity tracking)
* Gesture recognition (swipe patterns)
* Environmental compensation (temperature/humidity)

Example: Dual-Layer LED Display
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here's a complete feature request example you can use as a template:

.. code-block:: text

   /generate-feature-with-coding-agent Dual-Layer LED Display with Zone-Based Guidance

   Problem:
   The current display (REQ_DSP_3, REQ_DSP_4, REQ_DSP_5) shows only the current position with a green LED and warnings (red) when too close or too far. However, drivers have no indication WHERE the ideal parking position is located and no directional guidance whether to move forward or backward.

   Solution: 5 Zones with Dual-Layer Display System

   Zone Definition (percentage of LED strip):
   - Zone 0 "Emergency": Below minimum distance threshold (< dist_min_mm)
   - Zone 1 "Too Close": First 20% of LEDs (dist_min_mm to 20%)
   - Zone 2 "Ideal": Next 20% of LEDs (20% to 40%)
   - Zone 3 "Too Far": Remaining 60% of LEDs (40% to dist_max_mm)
   - Zone 4 "Out of Range": Beyond maximum distance threshold (> dist_max_mm)

   Layer System:

   Upper Layer (Position Indicator):
   - Single WHITE LED shows current vehicle position (when measurement valid)

   Lower Layer (Zone-Based Visualization):

   Zone 0 - Emergency (measurement < dist_min_mm):
   - All Zone 1 LEDs BLINK RED (1 Hz, 500ms on/off) → "DANGER! Move backward!"
   - Ideal zone at 5% RED
   - No position indicator (invalid measurement)

   Zone 1 - Too Close (measurement in Zone 1):
   - All Zone 1 LEDs ORANGE (background)
   - Two BLACK LEDs moving toward ideal zone → "back up!"
   - Ideal zone at 5% RED

   Zone 2 - Ideal (measurement in Zone 2):
   - Ideal zone at 100% RED → "STOP HERE!" (like traffic light)
   - Zone 1 and 3 OFF

   Zone 3 - Too Far (measurement in Zone 3):
   - Two GREEN LEDs at 5% brightness moving toward ideal zone → "move forward!"
   - Ideal zone at 5% GREEN
   - Zone 1 OFF

   Zone 4 - Out of Range (measurement > dist_max_mm):
   - Last LED (led_count-1) at 5% BLUE → "too far, no valid measurement"
   - Ideal zone at 5% GREEN
   - No position indicator

   Technical:
   - Animation speed: ~100ms per step
   - Blink frequency: 1 Hz (500ms on, 500ms off)
   - Position layer (white LED) renders on top of all zone layers

**How to use this example:**

1. Copy the text above
2. Use the generate-feature prompt (`.github/prompts/generate-feature-with-coding-agent.prompt.md`)
3. Paste it when prompted for the feature request
4. The prompt will automatically create a GitHub issue with full traceability
5. Assign to Coding Agent or work through it yourself

Implementation Guidelines
~~~~~~~~~~~~~~~~~~~~~~~~~

For any custom feature:

1. **Write clear issue** describing desired behavior
2. **Include acceptance criteria** (how to verify it works)
3. **Reference existing docs** (requirements, design, API)
4. **Specify traceability** (which requirements affected)
5. **Assign to Coding Agent** and monitor PR
6. **Test thoroughly** on hardware and/or QEMU
7. **Document your work** (update README if significant)

.. tip::

   **Start Small:** Begin with simple features and gradually increase complexity. 
   GitHub Coding Agent handles incremental improvements well!

Tips for Success
----------------

Best Practices
~~~~~~~~~~~~~~

**Clear Issue Descriptions:**

* ✅ Specific, measurable requirements
* ✅ Reference existing documentation
* ✅ Include acceptance criteria
* ✅ Mention affected components
* ❌ Vague requests without context

**Effective Prompts:**

* ✅ "Update requirements and design docs"
* ✅ "Maintain traceability to REQ_DSP_*"
* ✅ "Follow ESP32 coding standards"
* ✅ "Test on hardware and QEMU"
* ❌ "Just make it work"

**Review Process:**

* ✅ Read all changed files carefully
* ✅ Verify requirements↔design↔code links
* ✅ Test on real hardware (not just QEMU)
* ✅ Check for memory leaks or resource issues
* ✅ Validate timing and real-time behavior

Debugging Tips
~~~~~~~~~~~~~~

**Build Errors:**

* Check component dependencies in ``CMakeLists.txt``
* Verify all header includes are correct
* Run ``idf.py fullclean`` then rebuild

**Runtime Issues:**

* Enable verbose logging: ``idf.py menuconfig`` → Component config → Log output
* Use ``ESP_LOGI/LOGD`` liberally in code
* Monitor heap usage: ``esp_get_free_heap_size()``
* Check FreeRTOS task stack sizes

**Hardware Problems:**

* Verify wiring matches pin configuration in code
* Check power supply sufficient for LEDs
* Test components individually before integration
* Use logic analyzer or oscilloscope if available

Common Pitfalls
~~~~~~~~~~~~~~~

❌ **Blocking in Tasks:**

Don't use ``vTaskDelay()`` in measurement tasks—use timers!

❌ **Forgetting Documentation:**

Always update requirements and design docs with code changes.

❌ **Ignoring Memory Limits:**

ESP32 has limited RAM—monitor heap carefully!

❌ **Not Testing on Hardware:**

QEMU is great but doesn't catch timing issues or hardware quirks.

Getting Help
------------

**During Workshop:**

* Ask facilitator or mentor
* Collaborate with other participants
* Use GitHub Copilot Chat for quick questions
* Check :doc:`../90_guides/index` for detailed guides

**After Workshop:**

* `GitHub Issues <https://github.com/enthali/esp32-distance/issues>`_ - Bug reports
* `GitHub Discussions <https://github.com/enthali/esp32-distance/discussions>`_ - Q&A
* Main documentation: :doc:`../index`
* Requirements: :doc:`../11_requirements/index`
* Design specs: :doc:`../12_design/index`

Share Your Work
---------------

Completed an exercise? Share it with the community!

**GitHub Discussions:**

Post your implementation, challenges faced, and lessons learned.

**Pull Requests:**

If you implemented something useful, contribute it back:

1. Fork the repository (if not already)
2. Create feature branch
3. Implement and document
4. Submit PR with description
5. Maintainers will review

**Social Media:**

Share your project on Twitter/X, LinkedIn, or Reddit with:

* Photos/videos of working hardware
* Code snippets or architecture diagrams
* Lessons learned and tips
* Tag ``#ESP32`` ``#IoT`` ``#GitHubCopilot``

What's Next?
------------

After completing exercises:

* **Explore Architecture:** :doc:`../03_architecture/index`
* **Read API Documentation:** :doc:`../21_api/index`
* **Study Requirements:** :doc:`../11_requirements/index`
* **Review Design Specs:** :doc:`../12_design/index`
* **Build Your Own Project:** Use this as a template!

.. tip::

   **Template Project:** This repository is designed as a template! 
   Fork it and build your own ESP32 IoT projects with the same structure, 
   documentation methodology, and development workflow.

---

**Have fun experimenting!** 🚀 **Happy coding!** 💻

.. note::

   **Feedback Welcome:** Found an issue or have suggestions for exercises? 
   Open an issue or discussion on GitHub!
