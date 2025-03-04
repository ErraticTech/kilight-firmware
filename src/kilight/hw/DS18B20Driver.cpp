/**
 * DS18B20Driver.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/DS18B20Driver.h"

#include "kilight/util/MathUtil.h"

namespace kilight::hw {
    DS18B20Driver::DS18B20Driver(DS2485Driver* const driver) :
        OneWireDevice(driver) {
    }

    int16_t DS18B20Driver::currentTemperature() const {
        return static_cast<int16_t>(static_cast<float>(m_scratchpad.temperature) * 100 / 16);
    }

    bool DS18B20Driver::startRequestTemperatureConversion() const {
        return driver()->startOneWireWriteBlockData(convert_temperature_command_t{address()});
    }

    bool DS18B20Driver::completeRequestTemperatureConversion() const {
        return driver()->completeOneWireWriteBlock();
    }

    bool DS18B20Driver::startReadScratchpadCommand() const {
        return driver()->startOneWireWriteBlockData(read_scratchpad_command_t{address()});
    }

    bool DS18B20Driver::completeReadScratchpadCommand() const {
        return driver()->completeOneWireWriteBlock();
    }

    bool DS18B20Driver::startReadScratchpad() const {
        return driver()->startOneWireReadBlock(sizeof(scratchpad_t));
    }

    bool DS18B20Driver::completeReadScratchpad() {
        if (!driver()->completeOneWireReadBlock(&m_scratchpad)) {
            return false;
        }
        std::array<std::byte, sizeof(scratchpad_t) - sizeof(scratchpad_t::crc)> buffer {};
        memcpy(buffer.begin(), &m_scratchpad, buffer.size());
        if (uint8_t const calculatedCRC = util::MathUtil::crc8(std::span<std::byte const>{buffer});
            calculatedCRC != m_scratchpad.crc) {
            DEBUG("Temperature CRC mismatch (calc: {} / received: {}), retrying", calculatedCRC, m_scratchpad.crc);
        } else {
            onTemperatureReady(currentTemperature());
        }
        return true;
    }

    bool DS18B20Driver::startCopyScratchpadCommand() const {
        return driver()->startOneWireWriteBlockData(copy_scratchpad_command_t{address()});
    }

    bool DS18B20Driver::completeCopyScratchpadCommand() const {
        return driver()->completeOneWireWriteBlock();
    }
}
