idf_component_register(
    SRCS "display_logic.c"
    INCLUDE_DIRS "."
    REQUIRES 
        led_controller 
        distance_sensor 
        config_manager
        freertos
    PRIV_REQUIRES 
        main
)
