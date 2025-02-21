
add_executable(kilight-firmware
        main.cpp
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
        kilight/core/Alarm.h
        kilight/conf/HardwareConfig.h
        kilight/conf/HardwareConfig.cpp
        kilight/hw/I2CDevice.h
        kilight/hw/I2CDevice.cpp
        kilight/hw/DS2485Driver.h
        kilight/hw/DS2485Driver.cpp
        kilight/hw/DS18B20Driver.h
        kilight/hw/DS18B20Driver.cpp
        kilight/hw/OneWireDevice.cpp
        kilight/hw/OneWireDevice.h
        kilight/hw/OneWireSubsystem.cpp
        kilight/hw/OneWireSubsystem.h
        kilight/hw/TemperatureSensor.cpp
        kilight/hw/TemperatureSensor.h
        kilight/hw/onewire_address.h
        kilight/hw/TMP1826Driver.cpp
        kilight/hw/TMP1826Driver.h
        kilight/core/CriticalSection.h
        kilight/core/CriticalSection.cpp
        kilight/output/LightSubsystem.h
        kilight/output/LightSubsystem.cpp
        kilight/output/output_data.h
        kilight/storage/save_data.h
        kilight/storage/StorageSubsystem.h
        kilight/storage/StorageSubsystem.cpp
        kilight/com/wifi_data.h
)

target_compile_options(kilight-firmware PRIVATE
        -Wall
        -Wextra
)

target_include_directories(kilight-firmware PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}/sdk-config"
)

target_link_libraries(kilight-firmware PUBLIC
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

target_compile_definitions(kilight-firmware PUBLIC
        PICO_FLASH_ASSUME_CORE1_SAFE=1
)

target_link_options(kilight-firmware PRIVATE
        LINKER:--print-memory-usage
)

pico_enable_stdio_usb(kilight-firmware OFF)
pico_enable_stdio_uart(kilight-firmware ON)
pico_add_extra_outputs(kilight-firmware)

message(STATUS "Generating project config file")

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/ProjectConfig.cpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/ProjectConfig.cpp"
        @ONLY
)

message(STATUS "Generating wifi config file")

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/WifiConfig.cpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/WifiConfig.cpp"
        @ONLY
)

add_custom_target(kilight-firmware-generate-build-config
        COMMENT "Generating build config file"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/BuildConfig.cpp"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/BuildConfig.cpp.in"
        VERBATIM
        COMMAND ${CMAKE_COMMAND}
        -DCONFIG_INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/kilight/conf/BuildConfig.cpp.in
        -DCONFIG_OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/BuildConfig.cpp
        -P "${PROJECT_SOURCE_DIR}/cmake/GenerateBuildConfigFile.cmake"
)

target_sources(kilight-firmware PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/BuildConfig.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/ProjectConfig.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/kilight/conf/WifiConfig.cpp"
)

add_dependencies(kilight-firmware kilight-firmware-generate-build-config)
