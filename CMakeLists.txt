# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(dongley)

# --- Local Network OTA Target ---

# Fallback base version if not defined earlier in CMake
if(NOT PROJECT_VER)
    set(PROJECT_VER "1.0.0")
endif()

add_custom_target(ota
    DEPENDS app 
    COMMAND python3 -u ${CMAKE_SOURCE_DIR}/deps/happy/scripts/ota_push.py
        --bin ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
        --project ${CMAKE_PROJECT_NAME}
        --base-version ${PROJECT_VER}
        --broker "10.1.0.201"
        --user "puck1e80"
        --password "A9CeSm4MX7tcSMT"
        --topic "dongley_v1_001/ota_trigger/set"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}    
    COMMENT "Building and pushing OTA update to ESP32..."
    USES_TERMINAL
)