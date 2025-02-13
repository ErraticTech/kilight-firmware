/**
 * HardwareConfig.h
 *
 * @author Patrick Lavigne
 */
#pragma once

#include <cstdint>

namespace kilight::conf {

    class HardwareConfig {
    public:
        static uint64_t getUniqueID();

        HardwareConfig() = delete;
        ~HardwareConfig() = delete;
    };
}
