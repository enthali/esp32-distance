Display Logic API
=================

.. apicomponent:: Display Logic
   :id: API_COMP_DISPLAY_LOGIC
   :status: implemented
   :header_file: main/components/display_logic/display_logic.h
   :links: REQ_DSP_1, REQ_DSP_2, REQ_DSP_3, REQ_DSP_4
   
   **Brief Description**

   Business logic component that converts distance measurements into LED strip visualizations. Provides clean separation between hardware abstraction (components) and application logic.
   
   Key features:
   
   * Distance Range Mapping: Configurable range mapped linearly to configured LED count
   * LED Spacing: Approximately (max_distance - min_distance) / led_count cm per LED
   * Visual Feedback: Normal range (green/blue), error indicators (red), sensor timeout/error patterns
   * Real-time safe, FreeRTOS task-based architecture
   * Integration with config_manager, distance_sensor, and led_controller components
   
   **Architecture:**
   
   - Priority 3 FreeRTOS task for LED visualization (between sensor priority 6 and test priority 2)
   - Blocking reads from distance sensor queue (waits for new measurements)
   - Update rate matches distance sensor measurement rate
   - Real-time safe with proper task priorities
   - Input: distance_sensor_get_latest() API (blocking)
   - Output: led_controller APIs (led_set_pixel, led_clear_all, led_show)
   - Error handling: Visual indicators for all sensor error states

Lifecycle Functions
-------------------

.. apifunction:: display_logic_start
   :id: API_FUNC_DISPLAY_LOGIC_START
   :status: implemented
   :component: API_COMP_DISPLAY_LOGIC
   :api_signature: esp_err_t display_logic_start(void)
   :returns: ESP_OK on success, ESP_ERR_* on failure
   :parameters: None
   :links: REQ_DSP_1, REQ_DSP_2, REQ_DSP_3, REQ_DSP_4
   
   Start the display logic system. Obtains configuration from config_manager, initializes hardware, and starts the display task. No separate init/start lifecycle needed.

   **Signature:**

   .. code-block:: c

      esp_err_t display_logic_start(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Display logic task started successfully
   * ``ESP_ERR_INVALID_STATE`` - Task already running, or required components not initialized
   * ``ESP_FAIL`` - Failed to create display logic task
   * ``ESP_ERR_*`` - Other error cases from underlying components

   .. note::
      - Single entry point - handles initialization and task startup
      - Task runs on core 1 with 4KB stack at priority 3
      - Task blocks on distance_sensor_get_latest() until new data arrives
      - Configuration obtained from config_manager API
      - Thread-safe: Task-based, no global state exposed
      - See requirements: REQ_DSP_1, REQ_DSP_2, REQ_DSP_3, REQ_DSP_4

