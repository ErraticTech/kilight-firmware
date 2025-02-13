/**
 * OneWireDevice.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/OneWireDevice.h"

#include <cassert>


namespace kilight::hw {
    OneWireDevice::OneWireDevice(DS2485Driver * const driver) : m_driver(driver) {
        assert(driver != nullptr);
    }

    DS2485Driver::onewire_address_t OneWireDevice::address() const {
        return m_address;
    }

    void OneWireDevice::setAddress(DS2485Driver::onewire_address_t const address) {
        m_address = address;
    }

    DS2485Driver* OneWireDevice::driver() const {
        // Make sure we never try to do anything with the driver before the address is set
        assert(address() != 0);
        return m_driver;
    }
} // hw
// kilight
