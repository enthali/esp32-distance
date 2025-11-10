idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        nvs_flash
        esp_timer
        freertos
        driver
        # Distance sensor system components
        led_controller
        distance_sensor
        startup_tests
        display_logic
        # Web interface and configuration
        config_manager
        web_server
        netif_uart_tunnel
        # Optional: Uncomment when HTTPS is needed
        # cert_handler
)