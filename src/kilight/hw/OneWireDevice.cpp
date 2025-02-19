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

    onewire_address_t const & OneWireDevice::address() const {
        return m_address;
    }

    void OneWireDevice::setAddress(onewire_address_t const & address) {
        m_address = address;
    }

    DS2485Driver* OneWireDevice::driver() const {
        // Make sure we never try to do anything with the driver before the address is set
        assert(address());
        return m_driver;
    }
} // hw
// kilight
