idf_component_register(
    SRCS 
        "led_running_test.c"
    INCLUDE_DIRS "."
    REQUIRES 
        led_controller
        driver
        freertos
)
