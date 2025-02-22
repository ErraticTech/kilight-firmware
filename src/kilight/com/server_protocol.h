/**
 * server_protocol.h
 *
 * @author Patrick Lavigne
 */

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <compare>

#include <mpf/util/macros.h>
#include <mpf/types/FixedPackedString.h>

#include "kilight/conf/HardwareConfig.h"
#include "kilight/conf/ProjectConfig.h"
#include "kilight/output/rgbcw_color.h"

namespace kilight::com {

    enum class OutputIdentifier: uint8_t {
        Invalid = 0,
        OutputA = 1
    };

    enum class RequestType : uint8_t {
        Invalid = 0,
        WriteRequest = 1,
        ReadRequest = 2,
        SystemInfoRequest = 3
    };

    enum class ResponseType : uint8_t {
        Invalid = 0,
        StateData = 1,
        SystemInfo = 2
    };

    struct PACKED write_request_t {
        OutputIdentifier outputId : 8 = OutputIdentifier::Invalid;

        output::rgbcw_color_t color = {};

        uint8_t brightness : 8 = 0;

        bool on : 8 = false;
    };

    struct PACKED output_state_t {
        output::rgbcw_color_volatile_t color = {};

        uint8_t volatile brightness : 8 = 0;

        bool volatile on : 8 = false;

        uint16_t volatile current : 16 = 0;

        constexpr auto operator<=>(output_state_t const& other) const noexcept = default;

        output_state_t& operator=(write_request_t const& writeRequest) {
            color = writeRequest.color;
            brightness = writeRequest.brightness;
            on = writeRequest.on;
            return *this;
        }
    };

    struct PACKED state_data_t {
        output_state_t outputA = {};

        int16_t volatile driverTemperature : 16 = 0;

        int16_t volatile powerSupplyTemperature : 16 = 0;

        uint16_t volatile fanRPM : 16 = 0;

        uint16_t volatile fanOutputPerThou : 16 = 0;

        constexpr auto operator<=>(state_data_t const& other) const noexcept = default;
    };


    struct PACKED system_info_t {
        static constexpr uint8_t HardwareIdStringMaxSize = 16;

        static constexpr uint8_t ModelStringMaxSize = 32;

        static constexpr uint8_t ManufacturerStringMaxSize = 32;

        static constexpr uint8_t FirmwareVersionStringMaxSize = 16;

        static constexpr uint8_t HardwareVersionStringMaxSize = 16;

        mpf::core::FixedPackedString<HardwareIdStringMaxSize> hardwareId{
                "{:016X}",
                conf::HardwareConfig::getUniqueID()
            };

        mpf::core::FixedPackedString<ModelStringMaxSize> model{
                conf::getProjectConfig().DeviceName
            };

        mpf::core::FixedPackedString<ManufacturerStringMaxSize> manufacturer{
                conf::getProjectConfig().ManufacturerName
            };

        mpf::core::FixedPackedString<FirmwareVersionStringMaxSize> firmwareVersion{
                conf::getProjectConfig().VersionString
            };

        mpf::core::FixedPackedString<HardwareVersionStringMaxSize> hardwareVersion{
                conf::getProjectConfig().HardwareVersionString
            };
    };

    struct PACKED response_header_t {
        uint8_t messageLength : 8 = 0;
        ResponseType responseType : 8 = ResponseType::Invalid;
    };

    template <typename ResponseBodyT, ResponseType responseType>
    struct PACKED response_t {
        response_header_t header{
                .messageLength = sizeof(ResponseBodyT) + 1,
                .responseType = responseType
            };

        ResponseBodyT body;

        response_t() = default;

        explicit response_t(ResponseBodyT const& body) :
            body(body) {
        }
    };

    using state_response_t = response_t<state_data_t, ResponseType::StateData>;

    using system_info_response_t = response_t<system_info_t, ResponseType::SystemInfo>;

}
