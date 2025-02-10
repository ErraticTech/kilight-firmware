/**
 * MathUtil
 *
 * @author Patrick Lavigne
 */


#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace kilight::util {

    class MathUtil {
    public:
        static uint16_t crc16(std::span<std::byte const> data);

        template<typename DataT>
        static uint16_t crc16(DataT const & data) {
            return crc16(std::span{reinterpret_cast<std::byte const*>(&data), sizeof(DataT)});
        }

        MathUtil() = delete;

        ~MathUtil() = delete;
    };

}
