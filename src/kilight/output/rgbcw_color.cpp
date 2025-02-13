/**
 * rgbcw_color.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/output/rgbcw_color.h"
#include "kilight/util/MathUtil.h"

using kilight::util::MathUtil;

namespace kilight::output {

    rgbcw_color_t::rgbcw_color_t(uint8_t const red,
                                 uint8_t const green,
                                 uint8_t const blue,
                                 uint8_t const coldWhite,
                                 uint8_t const warmWhite) :
        red(red),
        green(green),
        blue(blue),
        coldWhite(coldWhite),
        warmWhite(warmWhite) {
    }

    rgbcw_color_t::rgbcw_color_t(rgbcw_color_volatile_t const& other) :
        red(other.red),
        green(other.green),
        blue(other.blue),
        coldWhite(other.coldWhite),
        warmWhite(other.warmWhite) {
    }

    auto rgbcw_color_t::operator<=>(rgbcw_color_volatile_t const& other) const {
        return *this <=> rgbcw_color_t(other);
    }

    rgbcw_color_t& rgbcw_color_t::operator=(rgbcw_color_volatile_t const& other) {
        red = other.red;
        green = other.green;
        blue = other.blue;
        coldWhite = other.coldWhite;
        warmWhite = other.warmWhite;
        return *this;
    }

    void rgbcw_color_t::incrementTowards(rgbcw_color_t const& other) {
        red = MathUtil::incrementBetween(red, other.red);
        green = MathUtil::incrementBetween(green, other.green);
        blue = MathUtil::incrementBetween(blue, other.blue);
        coldWhite = MathUtil::incrementBetween(coldWhite, other.coldWhite);
        warmWhite = MathUtil::incrementBetween(warmWhite, other.warmWhite);
    }

    rgbcw_color_volatile_t::rgbcw_color_volatile_t(uint8_t const red,
                                                   uint8_t const green,
                                                   uint8_t const blue,
                                                   uint8_t const coldWhite,
                                                   uint8_t const warmWhite) :
        red(red),
        green(green),
        blue(blue),
        coldWhite(coldWhite),
        warmWhite(warmWhite) {
    }

    rgbcw_color_volatile_t::rgbcw_color_volatile_t(rgbcw_color_t const& other) :
        red(other.red),
        green(other.green),
        blue(other.blue),
        coldWhite(other.coldWhite),
        warmWhite(other.warmWhite) {
    }

    rgbcw_color_volatile_t& rgbcw_color_volatile_t::operator=(rgbcw_color_t const& other) {
        red = other.red;
        green = other.green;
        blue = other.blue;
        coldWhite = other.coldWhite;
        warmWhite = other.warmWhite;
        return *this;
    }

    void rgbcw_color_volatile_t::incrementTowards(rgbcw_color_volatile_t const& other) {
        red = MathUtil::incrementBetween(red, other.red);
        green = MathUtil::incrementBetween(green, other.green);
        blue = MathUtil::incrementBetween(blue, other.blue);
        coldWhite = MathUtil::incrementBetween(coldWhite, other.coldWhite);
        warmWhite = MathUtil::incrementBetween(warmWhite, other.warmWhite);
    }


}
