LED Controller API
==================

.. apicomponent:: LED Controller
   :id: API_COMP_LED_CONTROLLER
   :status: implemented
   :header_file: main/components/led_controller/led_controller.h
   :links: REQ_LED_1, REQ_LED_2, REQ_LED_3, REQ_LED_4, REQ_LED_5, REQ_CFG_1
   
   **Brief Description**

   Hardware abstraction layer for WS2812 addressable LED strips using ESP32 RMT peripheral. Provides low-level control with RAM buffer management for efficient updates, individual pixel control, and color utilities.
   
   Key features:
   
   * Individual pixel control (set/get/clear)
   * RAM buffer maintains current LED state
   * Manual update trigger for performance optimization
   * Configurable LED count and GPIO pin
   * Color utility functions
   
   **Architecture:**
   
   - RMT peripheral abstraction for WS2812 timing
   - RAM buffer for current LED state
   - Manual update (led_show) for performance
   - Configurable LED count and GPIO pin
   - Color utility functions for RGB and brightness

Lifecycle Functions
-------------------

.. apifunction:: led_controller_init
   :id: API_FUNC_LED_CONTROLLER_INIT
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_controller_init(gpio_num_t data_pin)
   :returns: ESP_OK on success, ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_ARG, ESP_ERR_*
   :parameters: data_pin
   :links: REQ_LED_1, REQ_LED_3, REQ_CFG_1
   
   Initialize the LED controller, configure GPIO and RMT, allocate RAM buffer, and load configuration.

   **Signature:**

   .. code-block:: c

      esp_err_t led_controller_init(gpio_num_t data_pin);

   **Parameters:**

   * ``data_pin`` - GPIO pin for WS2812 data line

   **Returns:**

   * ``ESP_OK`` - Initialization successful
   * ``ESP_ERR_INVALID_STATE`` - Config manager not initialized
   * ``ESP_ERR_INVALID_ARG`` - Invalid pin argument
   * ``ESP_ERR_*`` - Other configuration or hardware errors

   .. note::
      - See requirements: REQ_LED_1, REQ_LED_3, REQ_CFG_1
      - Allocates memory for LED buffer (3 bytes per LED)
      - Only one instance supported, uses RMT channel 0

.. apifunction:: led_controller_deinit
   :id: API_FUNC_LED_CONTROLLER_DEINIT
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_controller_deinit(void)
   :returns: ESP_OK on success, ESP_ERR_*
   :parameters: None
   :links: REQ_LED_1
   
   Deinitialize the LED controller, free memory, deinit RMT, and reset GPIO.

   **Signature:**

   .. code-block:: c

      esp_err_t led_controller_deinit(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Deinitialization successful
   * ``ESP_ERR_*`` - Failure

   .. note::
      - See requirements: REQ_LED_1
      - Frees all resources, resets hardware

Pixel Control Functions
-----------------------

.. apifunction:: led_set_pixel
   :id: API_FUNC_LED_CONTROLLER_SET_PIXEL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_set_pixel(uint16_t index, led_color_t color)
   :returns: ESP_OK on success, ESP_ERR_INVALID_ARG
   :parameters: index, color
   :links: REQ_LED_2, REQ_LED_3
   
   Set color of a specific LED pixel in RAM buffer.

   **Signature:**

   .. code-block:: c

      esp_err_t led_set_pixel(uint16_t index, led_color_t color);

   **Parameters:**

   * ``index`` - LED index (0 to led_count-1)
   * ``color`` - RGB color to set

   **Returns:**

   * ``ESP_OK`` - Pixel set successfully
   * ``ESP_ERR_INVALID_ARG`` - Index out of range

   .. note::
      - See requirements: REQ_LED_2, REQ_LED_3
      - Call led_show() to update physical LEDs

.. apifunction:: led_clear_pixel
   :id: API_FUNC_LED_CONTROLLER_CLEAR_PIXEL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_clear_pixel(uint16_t index)
   :returns: ESP_OK on success, ESP_ERR_INVALID_ARG
   :parameters: index
   :links: REQ_LED_2
   
   Clear a specific LED pixel (set to off/black).

   **Signature:**

   .. code-block:: c

      esp_err_t led_clear_pixel(uint16_t index);

   **Parameters:**

   * ``index`` - LED index (0 to led_count-1)

   **Returns:**

   * ``ESP_OK`` - Pixel cleared successfully
   * ``ESP_ERR_INVALID_ARG`` - Index out of range

   .. note::
      - See requirements: REQ_LED_2
      - Equivalent to led_set_pixel(index, LED_COLOR_OFF)

.. apifunction:: led_get_pixel
   :id: API_FUNC_LED_CONTROLLER_GET_PIXEL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: led_color_t led_get_pixel(uint16_t index)
   :returns: Current LED color, or LED_COLOR_OFF if index out of range
   :parameters: index
   :links: REQ_LED_2, REQ_LED_5
   
   Get current color of a specific LED pixel from RAM buffer.

   **Signature:**

   .. code-block:: c

      led_color_t led_get_pixel(uint16_t index);

   **Parameters:**

   * ``index`` - LED index (0 to led_count-1)

   **Returns:**

   * ``led_color_t`` - Current color, or LED_COLOR_OFF if out of range

   .. note::
      - See requirements: REQ_LED_2, REQ_LED_5
      - Reads from RAM buffer, not physical LED

.. apifunction:: led_clear_all
   :id: API_FUNC_LED_CONTROLLER_CLEAR_ALL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_clear_all(void)
   :returns: ESP_OK on success, ESP_ERR_*
   :parameters: None
   :links: REQ_LED_2
   
   Clear all LED pixels (set all to off/black) in RAM buffer.

   **Signature:**

   .. code-block:: c

      esp_err_t led_clear_all(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - All pixels cleared
   * ``ESP_ERR_*`` - Failure

   .. note::
      - See requirements: REQ_LED_2
      - Call led_show() to update physical LEDs

.. apifunction:: led_show
   :id: API_FUNC_LED_CONTROLLER_SHOW
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_show(void)
   :returns: ESP_OK on success, ESP_ERR_*
   :parameters: None
   :links: REQ_LED_2, REQ_LED_4
   
   Update physical LED strip with current RAM buffer.

   **Signature:**

   .. code-block:: c

      esp_err_t led_show(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Update successful
   * ``ESP_ERR_*`` - Failure

   .. note::
      - See requirements: REQ_LED_2, REQ_LED_4
      - Blocks until transmission complete (~1-2ms for 40 LEDs)

Color Utility Functions
-----------------------

.. apifunction:: led_color_rgb
   :id: API_FUNC_LED_CONTROLLER_COLOR_RGB
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: led_color_t led_color_rgb(uint8_t r, uint8_t g, uint8_t b)
   :returns: RGB color structure
   :parameters: r, g, b
   :links: REQ_LED_4
   
   Create RGB color from individual components.

   **Signature:**

   .. code-block:: c

      led_color_t led_color_rgb(uint8_t r, uint8_t g, uint8_t b);

   **Parameters:**

   * ``r`` - Red component (0-255)
   * ``g`` - Green component (0-255)
   * ``b`` - Blue component (0-255)

   **Returns:**

   * ``led_color_t`` - RGB color structure

   .. note::
      - See requirements: REQ_LED_4
      - Use for custom colors

.. apifunction:: led_color_brightness
   :id: API_FUNC_LED_CONTROLLER_COLOR_BRIGHTNESS
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: led_color_t led_color_brightness(led_color_t color, uint8_t brightness)
   :returns: Color with applied brightness scaling
   :parameters: color, brightness
   :links: REQ_LED_4
   
   Apply brightness scaling to a color.

   **Signature:**

   .. code-block:: c

      led_color_t led_color_brightness(led_color_t color, uint8_t brightness);

   **Parameters:**

   * ``color`` - Original color
   * ``brightness`` - Brightness factor (0=off, 255=full)

   **Returns:**

   * ``led_color_t`` - Color with brightness applied

   .. note::
      - See requirements: REQ_LED_4
      - Uses integer math, no floating point

Configuration/Status Functions
------------------------------

.. apifunction:: led_get_count
   :id: API_FUNC_LED_CONTROLLER_GET_COUNT
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: uint16_t led_get_count(void)
   :returns: Number of LEDs, or 0 if not initialized
   :parameters: None
   :links: REQ_LED_3, REQ_CFG_1
   
   Get configured LED count.

   **Signature:**

   .. code-block:: c

      uint16_t led_get_count(void);

   **Parameters:**

   * None

   **Returns:**

   * ``uint16_t`` - Number of LEDs, or 0 if not initialized

   .. note::
      - See requirements: REQ_LED_3, REQ_CFG_1
      - Returns value from configuration

.. apifunction:: led_is_initialized
   :id: API_FUNC_LED_CONTROLLER_IS_INITIALIZED
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: bool led_is_initialized(void)
   :returns: true if initialized, false otherwise
   :parameters: None
   :links: REQ_LED_1
   
   Check if LED controller is initialized.

   **Signature:**

   .. code-block:: c

      bool led_is_initialized(void);

   **Parameters:**

   * None

   **Returns:**

   * ``true`` - Initialized
   * ``false`` - Not initialized

   .. note::
      - See requirements: REQ_LED_1
      - Use before calling other API functions

.. apifunction:: led_get_all_colors
   :id: API_FUNC_LED_CONTROLLER_GET_ALL_COLORS
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: uint16_t led_get_all_colors(led_color_t* output_buffer, uint16_t max_count)
   :returns: Number of LEDs copied, or 0 on error
   :parameters: output_buffer, max_count
   :links: REQ_LED_5
   
   Get snapshot of all LED colors (thread-safe, matches last led_show).

   **Signature:**

   .. code-block:: c

      uint16_t led_get_all_colors(led_color_t* output_buffer, uint16_t max_count);

   **Parameters:**

   * ``output_buffer`` - Buffer to receive LED colors (must be >= led_count)
   * ``max_count`` - Maximum number of LEDs to copy

   **Returns:**

   * ``uint16_t`` - Number of LEDs copied, or 0 on error

   .. note::
      - See requirements: REQ_LED_5
      - Thread-safe, matches physical LED state

Data Types
----------

.. apistruct:: led_color_t
   :id: API_STRUCT_LED_COLOR
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :links: REQ_LED_4
   
   RGB color structure for LED pixel color.

   **Definition:**

   .. code-block:: c

      typedef struct {
          uint8_t red;
          uint8_t green;
          uint8_t blue;
      } led_color_t;

   **Fields:**

   * ``red`` - Red component (0-255)
   * ``green`` - Green component (0-255)
   * ``blue`` - Blue component (0-255)

