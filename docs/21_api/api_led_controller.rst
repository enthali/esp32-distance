LED Controller API
==================

.. apicomponent:: LED Controller
   :id: API_COMP_LED_CONTROLLER
   :status: implemented
   :header_file: main/components/led_controller/led_controller.h
   :links: REQ_LED_1, REQ_LED_2, REQ_LED_3, REQ_LED_4, REQ_LED_5, REQ_CFG_1
   
   **Brief Description**

   Hardware abstraction layer for WS2812 addressable LED strips using ESP32 RMT peripheral. Provides low-level control with RAM buffer management for efficient updates.

   Key features:
   
   * Individual pixel control (set/get/clear)
   * RAM buffer maintains current LED state
   * Manual update trigger for performance optimization
   * Configurable LED count and GPIO pin
   * Color utility functions

   **Architecture:**

   Uses ESP32 RMT peripheral for precise WS2812 timing. Maintains a RAM buffer for current LED state. Only one controller instance supported. Configuration is loaded from NVS via config_manager.

Data Types
----------

.. apistruct:: led_color_t
   :id: API_STRUCT_LED_COLOR
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :links: REQ_LED_2, REQ_LED_4
   
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

Lifecycle Functions
-------------------

.. apifunction:: led_controller_init
   :id: API_FUNC_LED_CONTROLLER_INIT
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_controller_init(gpio_num_t data_pin)
   :returns: ESP_OK on success, ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_ARG, ESP_ERR_*
   :parameters: data_pin (GPIO pin for WS2812 data line)
   :links: REQ_LED_1, REQ_LED_3, REQ_CFG_1

   Initialize the LED controller. Loads configuration from config_manager and allocates RAM buffer. Only one instance supported. Uses RMT channel 0.

   **Signature:**

   .. code-block:: c

      esp_err_t led_controller_init(gpio_num_t data_pin);

   **Parameters:**

   * ``data_pin`` - GPIO pin for WS2812 data line

   **Returns:**

   * ``ESP_OK`` - Initialization successful
   * ``ESP_ERR_INVALID_STATE`` - Config manager not initialized
   * ``ESP_ERR_INVALID_ARG`` - Invalid pin
   * ``ESP_ERR_*`` - Configuration or hardware error

   .. note::
      Allocates memory for LED buffer (3 bytes per LED). Only one instance supported. RMT channel 0 is used.

.. apifunction:: led_controller_deinit
   :id: API_FUNC_LED_CONTROLLER_DEINIT
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_controller_deinit(void)
   :returns: ESP_OK on success, ESP_ERR_*
   :parameters: None
   :links: REQ_LED_1

   Deinitialize the LED controller. Frees memory, deinitializes RMT, and resets GPIO.

   **Signature:**

   .. code-block:: c

      esp_err_t led_controller_deinit(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - Deinitialization successful
   * ``ESP_ERR_*`` - Failure during deinit

   .. note::
      Frees all allocated resources and resets hardware state.

Pixel Control Functions
-----------------------

.. apifunction:: led_set_pixel
   :id: API_FUNC_LED_SET_PIXEL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_set_pixel(uint16_t index, led_color_t color)
   :returns: ESP_OK on success, ESP_ERR_INVALID_ARG
   :parameters: index (LED index), color (RGB color)
   :links: REQ_LED_2, REQ_LED_4

   Set color of a specific LED pixel in RAM buffer. Call led_show() to update physical LEDs.

   **Signature:**

   .. code-block:: c

      esp_err_t led_set_pixel(uint16_t index, led_color_t color);

   **Parameters:**

   * ``index`` - LED index (0 to led_count-1)
   * ``color`` - RGB color to set

   **Returns:**

   * ``ESP_OK`` - Pixel color updated in buffer
   * ``ESP_ERR_INVALID_ARG`` - Index out of range

   .. note::
      Does not update physical LEDs until led_show() is called.

.. apifunction:: led_clear_pixel
   :id: API_FUNC_LED_CLEAR_PIXEL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_clear_pixel(uint16_t index)
   :returns: ESP_OK on success, ESP_ERR_INVALID_ARG
   :parameters: index (LED index)
   :links: REQ_LED_2

   Clear a specific LED pixel (turn off). Equivalent to led_set_pixel(index, LED_COLOR_OFF).

   **Signature:**

   .. code-block:: c

      esp_err_t led_clear_pixel(uint16_t index);

   **Parameters:**

   * ``index`` - LED index (0 to led_count-1)

   **Returns:**

   * ``ESP_OK`` - Pixel cleared
   * ``ESP_ERR_INVALID_ARG`` - Index out of range

   .. note::
      Does not update physical LEDs until led_show() is called.

.. apifunction:: led_get_pixel
   :id: API_FUNC_LED_GET_PIXEL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: led_color_t led_get_pixel(uint16_t index)
   :returns: Current color or LED_COLOR_OFF
   :parameters: index (LED index)
   :links: REQ_LED_2, REQ_LED_4

   Get current color of a specific LED pixel from RAM buffer.

   **Signature:**

   .. code-block:: c

      led_color_t led_get_pixel(uint16_t index);

   **Parameters:**

   * ``index`` - LED index (0 to led_count-1)

   **Returns:**

   * ``led_color_t`` - Current color, or LED_COLOR_OFF if index out of range

   .. note::
      Reads from RAM buffer, not physical LED.

.. apifunction:: led_clear_all
   :id: API_FUNC_LED_CLEAR_ALL
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_clear_all(void)
   :returns: ESP_OK on success, ESP_ERR_*
   :parameters: None
   :links: REQ_LED_2

   Clear all LED pixels (turn off all LEDs) in RAM buffer. Call led_show() to update physical LEDs.

   **Signature:**

   .. code-block:: c

      esp_err_t led_clear_all(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - All pixels cleared
   * ``ESP_ERR_*`` - Failure

   .. note::
      Does not update physical LEDs until led_show() is called.

.. apifunction:: led_show
   :id: API_FUNC_LED_SHOW
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: esp_err_t led_show(void)
   :returns: ESP_OK on success, ESP_ERR_*
   :parameters: None
   :links: REQ_LED_4

   Update physical LED strip with current RAM buffer using RMT peripheral. Blocks until transmission is complete.

   **Signature:**

   .. code-block:: c

      esp_err_t led_show(void);

   **Parameters:**

   * None

   **Returns:**

   * ``ESP_OK`` - LEDs updated successfully
   * ``ESP_ERR_*`` - Failure

   .. note::
      Blocks for ~1-2ms for 40 LEDs. Only function that updates visible LEDs.

Utility Functions
-----------------

.. apifunction:: led_color_rgb
   :id: API_FUNC_LED_COLOR_RGB
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: led_color_t led_color_rgb(uint8_t r, uint8_t g, uint8_t b)
   :returns: RGB color structure
   :parameters: r (red), g (green), b (blue)
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
      Utility for constructing custom colors.

.. apifunction:: led_color_brightness
   :id: API_FUNC_LED_COLOR_BRIGHTNESS
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: led_color_t led_color_brightness(led_color_t color, uint8_t brightness)
   :returns: Color with applied brightness
   :parameters: color (original color), brightness (0-255)
   :links: REQ_LED_4

   Apply brightness scaling to a color. Scales all RGB components by brightness factor.

   **Signature:**

   .. code-block:: c

      led_color_t led_color_brightness(led_color_t color, uint8_t brightness);

   **Parameters:**

   * ``color`` - Original color
   * ``brightness`` - Brightness factor (0=off, 255=full)

   **Returns:**

   * ``led_color_t`` - Color with brightness applied

   .. note::
      Uses integer math for efficiency.

Query and State Functions
-------------------------

.. apifunction:: led_get_count
   :id: API_FUNC_LED_GET_COUNT
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: uint16_t led_get_count(void)
   :returns: Number of LEDs or 0
   :parameters: None
   :links: REQ_LED_3

   Get configured LED count.

   **Signature:**

   .. code-block:: c

      uint16_t led_get_count(void);

   **Parameters:**

   * None

   **Returns:**

   * ``uint16_t`` - Number of LEDs, or 0 if not initialized

.. apifunction:: led_is_initialized
   :id: API_FUNC_LED_IS_INITIALIZED
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

   * ``true`` - Controller initialized
   * ``false`` - Not initialized

.. apifunction:: led_get_all_colors
   :id: API_FUNC_LED_GET_ALL_COLORS
   :status: implemented
   :component: API_COMP_LED_CONTROLLER
   :api_signature: uint16_t led_get_all_colors(led_color_t* output_buffer, uint16_t max_count)
   :returns: Number of LEDs copied or 0
   :parameters: output_buffer (buffer), max_count (buffer size)
   :links: REQ_LED_5

   Get snapshot of all LED colors (thread-safe). Returns stable snapshot matching last physical LED update.

   **Signature:**

   .. code-block:: c

      uint16_t led_get_all_colors(led_color_t* output_buffer, uint16_t max_count);

   **Parameters:**

   * ``output_buffer`` - Pointer to buffer to receive LED colors
   * ``max_count`` - Maximum number of LEDs to copy (buffer size)

   **Returns:**

   * ``uint16_t`` - Number of LEDs actually copied, or 0 on error

   .. note::
      Caller must allocate output_buffer with size >= led_count * sizeof(led_color_t). Thread-safe, may block briefly. Returns snapshot matching physical LED state.
