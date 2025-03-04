/**
 * save_data.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <mpf/util/macros.h>

#include "kilight/output/output_data.h"
#include "kilight/com/wifi_data.h"
#include "kilight/hw/onewire_address.h"

namespace kilight::storage {

    struct PACKED save_data_t {
        struct PACKED thermometer_addresses_t {
            hw::onewire_address_t powerSupply = {};

            hw::onewire_address_t outputA = {};

            #ifdef KILIGHT_HAS_OUTPUT_B
            hw::onewire_address_t outputB = {};
            #endif

            constexpr auto operator<=>(thermometer_addresses_t const &other) const noexcept = default;
        };

        com::wifi_data_t wifi = {};

        thermometer_addresses_t thermometerAddresses = {};

        output::output_data_t outputA = {};

        #ifdef KILIGHT_HAS_OUTPUT_B
        output::output_data_t outputB = {};
        #endif

        constexpr auto operator<=>(save_data_t const &other) const noexcept = default;
    };
}
