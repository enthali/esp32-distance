if(CONFIG_TARGET_EMULATOR)
    set(WEB_SRCS "web_server.c" "wifi_manager_sim.c")
else()
    set(WEB_SRCS "web_server.c" "wifi_manager.c")
endif()

idf_component_register(
    SRCS ${WEB_SRCS}
    INCLUDE_DIRS "."
    EMBED_FILES 
        "www/index.html"
        "www/wifi-setup.html" 
        "www/settings.html"
        "www/favicon.svg"
        "www/css/style.css"
        "www/js/app.js"
    REQUIRES 
        cert_handler 
        config_manager
        esp_wifi 
        esp_http_server 
        esp_https_server 
        esp_netif 
        nvs_flash 
        json
        netif_uart_tunnel
    PRIV_REQUIRES 
        main
)