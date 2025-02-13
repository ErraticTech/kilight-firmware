/**
 * rgbcw_color.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>

// ReSharper disable once CppUnusedIncludeDirective
#include <compare>

#include <mpf/util/macros.h>

namespace kilight::output {
    struct PACKED rgbcw_color_volatile_t;

    struct PACKED rgbcw_color_t {
        uint8_t red : 8 = 0;
        uint8_t green : 8 = 0;
        uint8_t blue : 8 = 0;
        uint8_t coldWhite : 8 = 0;
        uint8_t warmWhite : 8 = 0;

        rgbcw_color_t() = default;

        rgbcw_color_t(uint8_t red, uint8_t green, uint8_t blue, uint8_t coldWhite, uint8_t warmWhite);

        explicit rgbcw_color_t(rgbcw_color_volatile_t const& other);

        auto operator<=>(rgbcw_color_t const& other) const = default;

        auto operator<=>(rgbcw_color_volatile_t const& other) const;

        rgbcw_color_t& operator=(rgbcw_color_t const& other) = default;

        rgbcw_color_t& operator=(rgbcw_color_volatile_t const& other);

        void incrementTowards(rgbcw_color_t const& other);
    };

    struct PACKED rgbcw_color_volatile_t {
        uint8_t volatile red : 8 = 0;
        uint8_t volatile green : 8 = 0;
        uint8_t volatile blue : 8 = 0;
        uint8_t volatile coldWhite : 8 = 0;
        uint8_t volatile warmWhite : 8 = 0;

        rgbcw_color_volatile_t() = default;

        rgbcw_color_volatile_t(uint8_t red, uint8_t green, uint8_t blue, uint8_t coldWhite, uint8_t warmWhite);

        explicit rgbcw_color_volatile_t(rgbcw_color_t const& other);

        auto operator<=>(rgbcw_color_volatile_t const& other) const = default;

        rgbcw_color_volatile_t& operator=(rgbcw_color_volatile_t const& other) = default;

        rgbcw_color_volatile_t& operator=(rgbcw_color_t const& other);

        void incrementTowards(rgbcw_color_volatile_t const& other);
    };
}
