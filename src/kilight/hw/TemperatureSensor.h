/**
 * TemperatureSensor.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>
#include <functional>

namespace kilight::hw {
    class TemperatureSensor {
    public:
        using OnTemperatureReadyCallbackT = std::function<void(int16_t)>;

        TemperatureSensor() = default;

        virtual ~TemperatureSensor() = default;

        [[nodiscard]]
        OnTemperatureReadyCallbackT const & onTemperatureReadyCallback() const;

        template<typename FuncT>
        void setOnTemperatureReadyCallback(FuncT && callback) {
            m_onTemperatureReadyCallback = std::forward<FuncT>(callback);
        }

        [[nodiscard]]
        virtual int16_t currentTemperature() const = 0;

    protected:
        void onTemperatureReady(int16_t temperature) const;

    private:
        OnTemperatureReadyCallbackT m_onTemperatureReadyCallback;
    };
}
