/**
 * WifiConfig.h
 *
 * @author Patrick Lavigne
 */
#pragma once

#include <string>
#include <cstdint>

namespace kilight::conf {
    struct wifi_config_t {
        std::string_view const SSID;
        std::string_view const Password;
        uint16_t const ListenPort;
    };

    wifi_config_t const & getWifiConfig();
}
