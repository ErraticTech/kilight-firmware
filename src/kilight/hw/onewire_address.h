/**
 * onewire_address.h
 *
 * @author Patrick Lavigne
 */

#pragma once
#include <cstdint>
// ReSharper disable once CppUnusedIncludeDirective
#include <compare>
#include <format>

#include <mpf/util/macros.h>

namespace kilight::hw {

    struct PACKED onewire_address_t {
        onewire_address_t() = default;

        // ReSharper disable once CppNonExplicitConversionOperator
        // NOLINTNEXTLINE(*-explicit-constructor)
        explicit(false) onewire_address_t(uint64_t const value) :
            m_value(value) {
        }

        // ReSharper disable once CppNonExplicitConversionOperator
        // NOLINTNEXTLINE(*-explicit-constructor)
        explicit(false) operator uint64_t() const {
            return m_value;
        }

        // ReSharper disable once CppNonExplicitConversionOperator
        // NOLINTNEXTLINE(*-explicit-constructor)
        explicit(false) operator bool() const {
            return m_value != 0ULL;
        }

        onewire_address_t& operator=(uint64_t const value) {
            m_value = value;
            return *this;
        }

        auto operator<=>(onewire_address_t const& other) const = default;

        [[nodiscard]]
        uint8_t deviceFamily() const {
            return static_cast<uint8_t>(m_value & 0xFF);
        }

        [[nodiscard]]
        uint64_t uniqueAddress() const {
            return (m_value & 0x00FFFFFFFFFF00ULL) >> 8;
        }

        [[nodiscard]]
        uint8_t crc() const {
            return static_cast<uint8_t>(m_value >> 56);
        }

    private:
        uint64_t m_value : 64 = 0;
    };

}

template <class CharT>
struct std::formatter<kilight::hw::onewire_address_t, CharT> {
    bool asParts = false;

    constexpr typename basic_format_parse_context<CharT>::iterator parse(basic_format_parse_context<CharT>& ctx) {
        auto iter = ctx.begin();

        if (iter == ctx.end()) {
            return iter;
        }

        if (*iter == '#') {
            asParts = true;
            ++iter;
        }

        if (iter != ctx.end() && *iter != '}') {
            return iter;
            __format::__failed_to_parse_format_spec();
        }

        return iter;
    }

    template <typename OutT>
    typename basic_format_context<OutT, CharT>::iterator format(kilight::hw::onewire_address_t const& address,
                                                                basic_format_context<OutT, CharT>& ctx) const {
        if (asParts) {
            return std::format_to(ctx.out(),
                                  "Family: {:02X} / UID: {:012X} / CRC: {:02X}",
                                  address.deviceFamily(),
                                  address.uniqueAddress(),
                                  address.crc());
        }
        return std::format_to(ctx.out(), "{:016X}", static_cast<uint64_t>(address));
    }
};
