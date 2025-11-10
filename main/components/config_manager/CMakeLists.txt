# =============================================================================
# COMPONENT REGISTRATION
# =============================================================================

idf_component_register(
    SRCS 
        "config_manager.c"
        "config_factory_generated.c"
    INCLUDE_DIRS "."
    REQUIRES 
        nvs_flash 
        esp_system
        json
    EMBED_FILES
        "config_schema.json"
)

# =============================================================================
# CODE GENERATION
# =============================================================================

# Find Python interpreter (must be after idf_component_register)
find_package(Python REQUIRED COMPONENTS Interpreter)

# Python script to generate factory defaults from JSON schema
set(GENERATOR_SCRIPT "${CMAKE_SOURCE_DIR}/tools/config/generate_config_factory.py")
set(CONFIG_SCHEMA "${CMAKE_CURRENT_SOURCE_DIR}/config_schema.json")
set(GENERATED_CODE "${CMAKE_CURRENT_SOURCE_DIR}/config_factory_generated.c")

# Add custom target to generate factory defaults
add_custom_target(
    generate_config_factory
    COMMAND ${Python_EXECUTABLE} ${GENERATOR_SCRIPT} ${CONFIG_SCHEMA} ${GENERATED_CODE}
    DEPENDS ${GENERATOR_SCRIPT} ${CONFIG_SCHEMA}
    COMMENT "Generating config factory defaults from JSON schema..."
    BYPRODUCTS ${GENERATED_CODE}
    VERBATIM
)

# Ensure code generation happens before building config_manager
add_dependencies(${COMPONENT_LIB} generate_config_factory)
