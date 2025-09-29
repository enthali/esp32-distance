idf_component_register(SRCS "main.c" 
                    INCLUDE_DIRS "."
                    REQUIRES led_controller distance_sensor cert_handler startup_tests web_server config_manager display_logic esp_wifi esp_http_server esp_https_server esp_netif nvs_flash json esp_gdbstub)