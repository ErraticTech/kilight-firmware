
add_library(kilight-firmware-lib STATIC
        sdk-config/lwipopts.h
        kilight/KiLight.h
        kilight/KiLight.cpp
        kilight/conf/BuildConfig.h
        kilight/conf/ProjectConfig.h
        kilight/conf/WifiConfig.h
        kilight/conf/WifiConfig.cpp.in
        kilight/util/MathUtil.cpp
        kilight/util/MathUtil.h
        kilight/hw/SysClock.h
        kilight/hw/SysClock.cpp
        kilight/core/LogSink.h
        kilight/core/LogSink.cpp
        kilight/hw/ADC.h
        kilight/hw/ADC.cpp
        kilight/hw/Pin.h
        kilight/hw/Pin.cpp
        kilight/hw/SystemPins.h
        kilight/hw/SystemPins.cpp
        kilight/com/server_protocol.h
        kilight/com/server_protocol.cpp
        kilight/com/WifiSubsystem.h
        kilight/com/WifiSubsystem.cpp
        kilight/output/rgbcw_color.h
        kilight/output/rgbcw_color.cpp
        kilight/core/Alarm.h
        kilight/conf/HardwareConfig.h
        kilight/conf/HardwareConfig.cpp
)

target_compile_options(kilight-firmware-lib PRIVATE
        -Wall
        -Wextra
)

target_include_directories(kilight-firmware-lib PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}/sdk-config"
)

target_link_libraries(kilight-firmware-lib PUBLIC
        pico_stdlib
        pico_lwip
        pico_lwip_mdns
        pico_cyw43_arch_lwip_threadsafe_background
        pico_flash
        pico_unique_id
        hardware_irq
        hardware_dma
        hardware_pwm
        hardware_i2c
        hardware_adc
        hardware_watchdog
        hardware_flash
        micro-program-framework
)

target_compile_definitions(kilight-firmware-lib PUBLIC
        PICO_FLASH_ASSUME_CORE1_SAFE=1
)

pico_enable_stdio_usb(kilight-firmware-lib OFF)
pico_enable_stdio_uart(kilight-firmware-lib ON)

message(STATUS "Generating project config file")

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/ProjectConfig.cpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/ProjectConfig.cpp"
        @ONLY
)

add_custom_command(COMMENT "Generating build config file"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/BuildConfig.cpp.in"
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/BuildConfig.cpp"
        COMMAND ${CMAKE_COMMAND}
        -DCONFIG_INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/BuildConfig.cpp.in
        -DCONFIG_OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/BuildConfig.cpp
        -P "${PROJECT_SOURCE_DIR}/cmake/GenerateBuildConfigFile.cmake"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
)

message(STATUS "Generating wifi config file")

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/WifiConfig.cpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/WifiConfig.cpp"
        @ONLY
)

target_sources(kilight-firmware-lib PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/BuildConfig.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/ProjectConfig.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/WifiConfig.cpp"
)

add_executable(kilight-firmware
        main.cpp
)

target_link_libraries(kilight-firmware PUBLIC kilight-firmware-lib)

target_compile_options(kilight-firmware PRIVATE
        -Wall
        -Wextra
)

target_link_options(kilight-firmware PRIVATE
        -Wl,--print-memory-usage
)

pico_add_extra_outputs(kilight-firmware)
