# Configure requirements based on build target
if(CONFIG_TARGET_EMULATOR)
    # QEMU simulator build
    set(MAIN_REQUIRES
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
        # QEMU-only networking components
        esp_netif
        lwip
        netif_uart_tunnel
    )
else()
    # Hardware build
    set(MAIN_REQUIRES
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
    )
endif()

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES ${MAIN_REQUIRES}
    # Optional: Uncomment when HTTPS is needed
    # cert_handler
)