/**
 * DS18B20Driver.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/DS18B20Driver.h"

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
        bool const result = driver()->completeOneWireReadBlock(&m_scratchpad);
        if (result) {
            onTemperatureReady(currentTemperature());
        }
        return result;
    }

    bool DS18B20Driver::startCopyScratchpadCommand() const {
        return driver()->startOneWireWriteBlockData(copy_scratchpad_command_t{address()});
    }

    bool DS18B20Driver::completeCopyScratchpadCommand() const {
        return driver()->completeOneWireWriteBlock();
    }
}
