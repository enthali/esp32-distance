# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Add main/components to the component search path
set(EXTRA_COMPONENT_DIRS "main/components")

# Project name
set(PROJECT_NAME "esp32-distance" CACHE STRING "Project name")

# Read version from VERSION file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION_STRING)
string(STRIP "${VERSION_STRING}" VERSION_STRING)
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" VERSION_MATCH "${VERSION_STRING}")
set(VERSION_MAJOR ${CMAKE_MATCH_1})
set(VERSION_MINOR ${CMAKE_MATCH_2})
set(VERSION_BUILD ${CMAKE_MATCH_3})

# Increment build number
math(EXPR VERSION_BUILD "${VERSION_BUILD} + 1")
set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}")

# Write updated version back to VERSION file
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" "${PROJECT_VERSION}")

# Set version for the project
message(STATUS "Project Version: ${PROJECT_VERSION} (Build: ${VERSION_BUILD})")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})