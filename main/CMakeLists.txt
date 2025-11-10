idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        nvs_flash
        esp_timer
        # Template components (web-based configuration)
        config_manager
        web_server
        netif_uart_tunnel
        # Optional: Uncomment when HTTPS is needed
        # cert_handler
)