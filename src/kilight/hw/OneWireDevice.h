/**
 * OneWireDevice.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include "kilight/hw/DS2485Driver.h"

namespace kilight::hw {

    class OneWireDevice {
    public:
        explicit OneWireDevice(DS2485Driver * driver);

        virtual ~OneWireDevice() = default;

        [[nodiscard]]
        onewire_address_t const & address() const;

        void setAddress(onewire_address_t const & address);

    protected:
        [[nodiscard]]
        DS2485Driver * driver() const;

    private:
        DS2485Driver * const m_driver;

        onewire_address_t m_address = 0;
    };

}
