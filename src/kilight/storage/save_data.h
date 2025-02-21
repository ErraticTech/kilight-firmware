/**
 * save_data.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <mpf/util/macros.h>

#include "kilight/output/output_data.h"
#include "kilight/com/wifi_data.h"

namespace kilight::storage {

    struct PACKED save_data_t {
        com::wifi_data_t wifi = {};

        output::output_data_t outputA = {};

        constexpr auto operator<=>(save_data_t const &other) const noexcept = default;
    };
}
