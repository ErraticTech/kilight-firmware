cmake_minimum_required(VERSION 3.24)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(Policies)

set(SERVER_LISTEN_PORT "10240" CACHE STRING "Port the TCP server will listen on")
set(DEVICE_NAME "KiLight Mono" CACHE STRING "Device name to report")
set(MANUFACTURER_NAME "Erratic.Tech" CACHE STRING "Manufacturer name to report")
set(HARDWARE_VERSION_MAJOR "1" CACHE STRING "Major revision of the hardware")
set(HARDWARE_VERSION_MINOR "0" CACHE STRING "Minor revision of the hardware")
set(HARDWARE_VERSION_PATCH "0" CACHE STRING "Patch revision of the hardware")
set(HARDWARE_VERSION "${HARDWARE_VERSION_MAJOR}.${HARDWARE_VERSION_MINOR}.${HARDWARE_VERSION_PATCH}" CACHE INTERNAL "")

set(PICO_BOARD_HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/boards/")
set(PICO_BOARD "kilight-mono-v1.0.x")
set(PICO_SDK_FETCH_FROM_GIT ON)
include(pico_sdk_import)

project(kilight-firmware VERSION 1.0.0
        DESCRIPTION "KiLight Firmware"
        LANGUAGES C CXX ASM)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

pico_sdk_init()

# A file that defines the cache variables WIFI_SSID and WIFI_PASSWORD to set what wifi network to connect to. Will
# eventually be replaced by dynamic configuration so it doesn't need to be compiled in.
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/Secrets.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/Secrets.cmake")
endif()

set(MPF_LOG_LEVEL "Debug" CACHE INTERNAL "")

add_subdirectory(lib/micro-program-framework)

add_subdirectory(src)

