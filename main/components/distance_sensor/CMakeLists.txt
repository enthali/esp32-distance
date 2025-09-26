# Conditional source selection based on build target
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "distance_sensor_sim.c")
else()
    set(COMPONENT_SRCS "distance_sensor.c")
endif()

idf_component_register(SRCS ${COMPONENT_SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES driver freertos esp_timer)
