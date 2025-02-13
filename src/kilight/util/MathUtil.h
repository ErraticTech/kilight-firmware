/**
 * MathUtil
 *
 * @author Patrick Lavigne
 */


#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>

namespace kilight::util {

    class MathUtil {
    public:
        static uint16_t crc16(std::span<std::byte const> data);

        template<typename DataT>
        static uint16_t crc16(DataT const & data) {
            std::array<std::byte, sizeof(DataT)> buffer;
            memcpy(buffer.begin(), &data, sizeof(DataT));
            return crc16(std::span{buffer});
        }

        /**
         * Returns the direction along the number line one must move second to bring it closer to first.
         *
         * @tparam NumberT Type of numbers to compare
         * @param first First number to compare
         * @param second Second number to compare
         * @return If first is equal to second, returns 0. If first is greater than second, returns 1. Otherwise, returns -1
         */
        template<typename NumberT>
        static int8_t deltaDirection(NumberT const first, NumberT const second) {
            if (first == second) {
                return 0;
            }
            if (first > second) {
                return 1;
            }
            return -1;
        }

        template<typename NumberT>
        static NumberT incrementBetween(NumberT const from, NumberT const to) {
            return static_cast<NumberT>(from + deltaDirection(to, from));
        }

        MathUtil() = delete;

        ~MathUtil() = delete;
    };

}
