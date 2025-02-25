/**
 * rgbcw_color.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>
#include <concepts>
#include <limits>
#include <format>

// ReSharper disable once CppUnusedIncludeDirective
#include <compare>

#include <mpf/util/macros.h>

#include <kilight/protocol/Color.h>

#include "kilight/util/MathUtil.h"

namespace kilight::output {
    template <std::unsigned_integral ColorDataT>
    struct PACKED rgbcw_color_base_t {
        ColorDataT red = 0U;
        ColorDataT green = 0U;
        ColorDataT blue = 0U;
        ColorDataT coldWhite = 0U;
        ColorDataT warmWhite = 0U;

        rgbcw_color_base_t() = default;

        constexpr rgbcw_color_base_t(ColorDataT const red,
                                     ColorDataT const green,
                                     ColorDataT const blue,
                                     ColorDataT const coldWhite,
                                     ColorDataT const warmWhite) :
            red(red),
            green(green),
            blue(blue),
            coldWhite(coldWhite),
            warmWhite(warmWhite) {
        }

        template <typename OtherColorT>
        constexpr explicit(false) rgbcw_color_base_t(OtherColorT const& other) :
            red(other.red),
            green(other.green),
            blue(other.blue),
            coldWhite(other.coldWhite),
            warmWhite(other.warmWhite) {
        }

        constexpr auto operator<=>(rgbcw_color_base_t const& other) const noexcept = default;

        template <typename OtherColorT>
        rgbcw_color_base_t& operator=(OtherColorT const& other) {
            red = other.red;
            green = other.green;
            blue = other.blue;
            coldWhite = other.coldWhite;
            warmWhite = other.warmWhite;
            return *this;
        }

        [[nodiscard]]
        protocol::Color toColor() const {
            protocol::Color color;
            color.set_red(red);
            color.set_green(green);
            color.set_blue(blue);
            color.set_coldWhite(coldWhite);
            color.set_warmWhite(warmWhite);
            return color;
        }

        template <typename OtherColorT>
        void incrementTowards(OtherColorT const& other) {
            red = util::MathUtil::incrementBetween(red, other.red);
            green = util::MathUtil::incrementBetween(green, other.green);
            blue = util::MathUtil::incrementBetween(blue, other.blue);
            coldWhite = util::MathUtil::incrementBetween(coldWhite, other.coldWhite);
            warmWhite = util::MathUtil::incrementBetween(warmWhite, other.warmWhite);
        }

        template <typename ReturnT = rgbcw_color_base_t, std::unsigned_integral IntermediateCalculationT = uint32_t>
        ReturnT scaledBy(ColorDataT const scaleFactor) const {
            return ReturnT{
                    static_cast<ColorDataT>(static_cast<IntermediateCalculationT>(red)
                                            * static_cast<IntermediateCalculationT>(scaleFactor)
                                            / std::numeric_limits<ColorDataT>::max()),
                    static_cast<ColorDataT>(static_cast<IntermediateCalculationT>(green)
                                            * static_cast<IntermediateCalculationT>(scaleFactor)
                                            / std::numeric_limits<ColorDataT>::max()),
                    static_cast<ColorDataT>(static_cast<IntermediateCalculationT>(blue)
                                            * static_cast<IntermediateCalculationT>(scaleFactor)
                                            / std::numeric_limits<ColorDataT>::max()),
                    static_cast<ColorDataT>(static_cast<IntermediateCalculationT>(coldWhite)
                                            * static_cast<IntermediateCalculationT>(scaleFactor)
                                            / std::numeric_limits<ColorDataT>::max()),
                    static_cast<ColorDataT>(static_cast<IntermediateCalculationT>(warmWhite)
                                            * static_cast<IntermediateCalculationT>(scaleFactor)
                                            / std::numeric_limits<ColorDataT>::max())
                };
        }
    };

    using rgbcw_color_t = rgbcw_color_base_t<uint8_t>;

    using rgbcw_color_volatile_t = rgbcw_color_base_t<uint8_t volatile>;


    template <class ColorT, class CharT>
    struct formatter_base {
        constexpr typename std::basic_format_parse_context<CharT>::iterator parse(std::basic_format_parse_context<CharT>& ctx) {
            auto iter = ctx.begin();

            if (iter == ctx.end()) {
                return iter;
            }

            if (iter != ctx.end() && *iter != '}') {
                return iter;
                std::__format::__failed_to_parse_format_spec();
            }

            return iter;
        }

        template <typename OutT>
        typename std::basic_format_context<OutT, CharT>::iterator format(ColorT const& colorData,
                                                                         std::basic_format_context<OutT, CharT>& ctx) const {

            return std::format_to(ctx.out(),
                                  "R: {:03d}, G: {:03d}, B: {:03d}, CW: {:03d}, WW: {:03d}",
                                  colorData.red,
                                  colorData.green,
                                  colorData.blue,
                                  colorData.coldWhite,
                                  colorData.warmWhite);
        }
    };
}

template <class CharT>
struct std::formatter<kilight::output::rgbcw_color_t, CharT>
    : public kilight::output::formatter_base<kilight::output::rgbcw_color_t, CharT> {};


template <class CharT>
struct std::formatter<kilight::output::rgbcw_color_volatile_t, CharT>
    : public kilight::output::formatter_base<kilight::output::rgbcw_color_volatile_t, CharT> {};
