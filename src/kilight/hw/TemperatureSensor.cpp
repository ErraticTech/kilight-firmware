/**
 * TemperatureSensor.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/TemperatureSensor.h"

namespace kilight::hw {
    TemperatureSensor::OnTemperatureReadyCallbackT const& TemperatureSensor::onTemperatureReadyCallback() const {
        return m_onTemperatureReadyCallback;
    }

    void TemperatureSensor::onTemperatureReady(int16_t const temperature) const {
        if (onTemperatureReadyCallback()) {
            onTemperatureReadyCallback()(temperature);
        }
    }
}
