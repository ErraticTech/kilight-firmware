/**
 * I2CDevice.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/I2CDevice.h"


#include <hardware/i2c.h>

namespace kilight::hw {

    I2CDevice::I2CDevice(uint8_t const address) :
        m_address(address) {
    }

    int I2CDevice::read(std::span<uint8_t> const& buffer, bool const noStop) const {
        return i2c_read_timeout_us(i2c_default, m_address, buffer.data(), buffer.size(), noStop, ReadTimeoutUs);
    }

    int I2CDevice::write(std::span<uint8_t const> const& buffer, bool const noStop) const {
        return i2c_write_timeout_us(i2c_default, m_address, buffer.data(), buffer.size(), noStop, WriteTimeoutUs);
    }

    int I2CDevice::readRegister(uint8_t const registerAddress,
                                std::span<uint8_t> const& buffer,
                                bool const noStop) const {
        if (int const writeResult = write(registerAddress, true); writeResult < 1) {
            return writeResult;
        }
        return i2c_read_timeout_us(i2c_default, m_address, buffer.data(), buffer.size(), noStop, ReadTimeoutUs);
    }
}
