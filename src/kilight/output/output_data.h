/**
 * output_data.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <mpf/util/macros.h>

#include "kilight/output/rgbcw_color.h"

namespace kilight::output {
    struct PACKED output_data_t {
        rgbcw_color_volatile_t color {};
        uint8_t volatile brightnessMultiplier = 0;
        bool volatile powerOn = false;

        output_data_t() = default;

        output_data_t(uint8_t const red,
                      uint8_t const green,
                      uint8_t const blue,
                      uint8_t const coldWhite,
                      uint8_t const warmWhite,
                      uint8_t const brightnessMultiplier,
                      bool const powerOn) :
            color(red, green, blue, coldWhite, warmWhite),
            brightnessMultiplier(brightnessMultiplier),
            powerOn(powerOn) {
        }

        constexpr auto operator<=>(output_data_t const& other) const noexcept = default;

        output_data_t& operator=(rgbcw_color_t const& other) {
            color = other;
            return *this;
        }

        [[nodiscard]]
        rgbcw_color_t getRGBCWColor() const {
            return rgbcw_color_t{color};
        }

        [[nodiscard]]
        rgbcw_color_t getRGBCWColorScaledToBrightness() const {
            return color.scaledBy<rgbcw_color_t>(brightnessMultiplier);
        }
    };
}
