idf_component_register(SRCS "distance_sensor.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver freertos esp_timer)
