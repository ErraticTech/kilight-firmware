/**
 * wifi_data.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>

#include <mpf/util/macros.h>
#include <mpf/types/FixedPackedString.h>

#include "kilight/conf/WifiConfig.h"

namespace kilight::com {
    static constexpr uint8_t MaxSSIDLength = 32;
    static constexpr uint8_t MaxPasswordLength = 63;

    struct PACKED wifi_data_t {
        using SSIDStringT = mpf::core::FixedPackedString<MaxSSIDLength, char volatile, uint8_t volatile>;

        using PasswordStringT = mpf::core::FixedPackedString<MaxPasswordLength, char volatile, uint8_t volatile>;

        SSIDStringT ssid{};

        PasswordStringT password{};

        wifi_data_t() = default;

        constexpr wifi_data_t(SSIDStringT const& ssid, PasswordStringT const& password) :
            ssid(ssid), password(password) {
        }

        explicit wifi_data_t(conf::wifi_config_t const& wifiConfig) noexcept :
            ssid(SSIDStringT::StringViewT{wifiConfig.SSID.begin(), wifiConfig.SSID.size()}),
            password(PasswordStringT::StringViewT{wifiConfig.Password.begin(), wifiConfig.Password.size()}) {
        }

        constexpr auto operator<=>(wifi_data_t const& other) const noexcept = default;
    };
}
