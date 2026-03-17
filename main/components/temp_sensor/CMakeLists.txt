# Conditional source selection based on build target
# When TARGET_EMULATOR is set, CONFIG_DISTANCE_SENSOR_SIMULATION is also set
# and we use the simulation implementation (SPEC_TEMP_ARCH_1)
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "temp_sensor_sim.c")
else()
    set(COMPONENT_SRCS "temp_sensor.c")
endif()

idf_component_register(SRCS ${COMPONENT_SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES driver freertos esp_timer config_manager distance_sensor)
