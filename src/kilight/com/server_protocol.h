/**
 * server_protocol.h
 *
 * @author Patrick Lavigne
 */

#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <compare>

#include <mpf/util/macros.h>

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
        OutputIdentifier outputId: 8 = OutputIdentifier::Invalid;

        output::rgbcw_color_t color = {};

        uint8_t brightness: 8 = 0;

        bool on: 8 = false;
    };

    struct PACKED output_state_t {
        output::rgbcw_color_volatile_t color = {};

        uint8_t volatile brightness: 8 = 0;

        bool volatile on: 8 = false;

        uint16_t volatile current: 16 = 0;

        constexpr auto operator<=>(output_state_t const &other) const noexcept = default;

        output_state_t & operator=(write_request_t const & writeRequest) {
            color = writeRequest.color;
            brightness = writeRequest.brightness;
            on = writeRequest.on;
            return *this;
        }
    };

    struct PACKED state_data_t {
        output_state_t outputA = {};

        int16_t volatile driverTemperature: 16 = 0;

        int16_t volatile powerSupplyTemperature: 16 = 0;

        uint16_t volatile fanRPM: 16 = 0;

        uint16_t volatile fanOutputPerThou: 16 = 0;

        constexpr auto operator<=>(state_data_t const &other) const noexcept = default;
    };

    struct PACKED system_info_t {
        NONSTRING
        char hardwareId[16] {};

        NONSTRING
        char model[32] {};

        NONSTRING
        char manufacturer[32] {};

        NONSTRING
        char firmwareVersion[16] {};

        NONSTRING
        char hardwareVersion[16] {};

        system_info_t();
    };

    struct PACKED response_header_t {
        uint8_t messageLength: 8 = 0;
        ResponseType responseType: 8 = ResponseType::Invalid;
    };

    template<typename ResponseBodyT, ResponseType responseType>
    struct PACKED response_t {
        response_header_t header {
            .messageLength = sizeof(ResponseBodyT) + 1,
            .responseType = responseType
        };

        ResponseBodyT body;

        response_t() = default;

        explicit response_t(ResponseBodyT const & body) : body(body) {}
    };

    using state_response_t = response_t<state_data_t, ResponseType::StateData>;

    using system_info_response_t = response_t<system_info_t, ResponseType::SystemInfo>;

}
