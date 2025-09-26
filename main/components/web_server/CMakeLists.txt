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
        "www/css/style.css"
        "www/js/app.js"
    REQUIRES 
        led_controller 
        distance_sensor 
        cert_handler 
        esp_wifi 
        esp_http_server 
        esp_https_server 
        esp_netif 
        nvs_flash 
        json
    PRIV_REQUIRES 
        main
)