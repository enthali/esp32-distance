# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Add main/components to the component search path
set(EXTRA_COMPONENT_DIRS "main/components")

# Project name
set(PROJECT_NAME "esp32-distance" CACHE STRING "Project name")

# Version from git tags (e.g. v2.1.0, v2.1.0-3-g1a2b3c4)
execute_process(
    COMMAND git describe --tags --match "v[0-9]*" --always --dirty
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE GIT_RESULT
)
if(GIT_RESULT EQUAL 0 AND GIT_VERSION MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})
    set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
else()
    set(PROJECT_VERSION "0.0.0")
endif()

message(STATUS "Project Version: ${PROJECT_VERSION} (git: ${GIT_VERSION})")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})