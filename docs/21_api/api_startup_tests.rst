Startup Tests API
=================

.. apicomponent:: Startup Tests
   :id: API_COMP_STARTUP_TESTS
   :status: implemented
   :header_file: main/components/startup_tests/led_running_test.h
   :links: REQ_STARTUP_1, REQ_STARTUP_2, REQ_STARTUP_3, REQ_LED_1, REQ_LED_2
   
   **Brief Description**

   Test functions for running light effects on the LED strip, used for startup visual feedback and hardware validation.
   
   Key features:
   
   * Sequential running light effect for boot validation
   * Multiple cycles and rainbow effect for advanced testing
   * Synchronous, blocking execution (no separate task)
   * Uses led_controller API for all LED operations
   
   **Architecture:**
   
   - Called after LED controller initialization in main()
   - Effects run synchronously, blocking until complete
   - No separate task/thread required; runs in main context
   - All LED operations use led_controller API

Test Functions
--------------

.. apifunction:: led_running_test_single_cycle
   :id: API_FUNC_STARTUP_TESTS_SINGLE_CYCLE
   :status: implemented
   :component: API_COMP_STARTUP_TESTS
   :api_signature: esp_err_t led_running_test_single_cycle(led_color_t color, uint32_t delay_ms)
   :returns: ESP_OK on success
   :parameters: color, delay_ms
   :links: REQ_STARTUP_2, REQ_LED_2
   
   Run a single cycle of running light effect (one LED moves across the strip).

   **Signature:**

   .. code-block:: c

      esp_err_t led_running_test_single_cycle(led_color_t color, uint32_t delay_ms);

   **Parameters:**

   * ``color`` - Color of the running LED
   * ``delay_ms`` - Delay between LED movements in milliseconds

   **Returns:**

   * ``ESP_OK`` - Test completed successfully

   .. note::
      - See requirements: REQ_STARTUP_2, REQ_LED_2
      - Synchronous, blocking execution

.. apifunction:: led_running_test_multiple_cycles
   :id: API_FUNC_STARTUP_TESTS_MULTIPLE_CYCLES
   :status: implemented
   :component: API_COMP_STARTUP_TESTS
   :api_signature: esp_err_t led_running_test_multiple_cycles(led_color_t color, uint32_t delay_ms, uint8_t cycles)
   :returns: ESP_OK on success
   :parameters: color, delay_ms, cycles
   :links: REQ_STARTUP_2, REQ_LED_2
   
   Run multiple cycles of running light effect.

   **Signature:**

   .. code-block:: c

      esp_err_t led_running_test_multiple_cycles(led_color_t color, uint32_t delay_ms, uint8_t cycles);

   **Parameters:**

   * ``color`` - Color of the running LED
   * ``delay_ms`` - Delay between LED movements in milliseconds
   * ``cycles`` - Number of complete cycles to run

   **Returns:**

   * ``ESP_OK`` - Test completed successfully

   .. note::
      - See requirements: REQ_STARTUP_2, REQ_LED_2
      - Synchronous, blocking execution

.. apifunction:: led_running_test_rainbow
   :id: API_FUNC_STARTUP_TESTS_RAINBOW
   :status: implemented
   :component: API_COMP_STARTUP_TESTS
   :api_signature: esp_err_t led_running_test_rainbow(uint32_t delay_ms, uint8_t cycles)
   :returns: ESP_OK on success
   :parameters: delay_ms, cycles
   :links: REQ_STARTUP_3, REQ_LED_2
   
   Run rainbow running light effect (color changes as it moves).

   **Signature:**

   .. code-block:: c

      esp_err_t led_running_test_rainbow(uint32_t delay_ms, uint8_t cycles);

   **Parameters:**

   * ``delay_ms`` - Delay between LED movements in milliseconds
   * ``cycles`` - Number of complete cycles to run

   **Returns:**

   * ``ESP_OK`` - Test completed successfully

   .. note::
      - See requirements: REQ_STARTUP_3, REQ_LED_2
      - Synchronous, blocking execution

