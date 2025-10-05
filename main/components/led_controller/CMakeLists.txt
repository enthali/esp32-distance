# Conditional source selection based on build target
if(CONFIG_TARGET_EMULATOR)
    set(COMPONENT_SRCS "led_controller_sim.c")
else()
    set(COMPONENT_SRCS "led_controller.c")
endif()

idf_component_register(SRCS ${COMPONENT_SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES driver log esp_timer)