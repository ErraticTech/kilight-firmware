/**
 * SysClock.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>

namespace kilight::hw {
    class SysClock {
    public:
        static constexpr uint32_t PLLVCOFrequency = 1536000000UL;
        static constexpr uint8_t PLLDiv1 = 6U;
        static constexpr uint8_t PLLDiv2 = 2U;
        static constexpr uint32_t Frequency = (PLLVCOFrequency / (PLLDiv1 * PLLDiv2));

        static void initialize();

        SysClock() = delete;
        ~SysClock() = delete;
    };

}
