/**
 * UserInterfaceSubsystem.h
 *
 * @author Patrick Lavigne
 */
#pragma once

#include <atomic>

#include <mpf/core/Subsystem.h>
#include <mpf/core/Logging.h>

#include "kilight/core/Alarm.h"
#include "kilight/hw/OneWireSubsystem.h"
#include "kilight/hw/SystemPins.h"
#include "kilight/storage/StorageSubsystem.h"

namespace kilight::ui {

    enum class NetworkStatusLEDState : uint8_t {
        Off = 0,
        Searching = 1,
        Connecting = 2,
        Connected = 3
    };

    class UserInterfaceSubsystem final : public mpf::core::Subsystem {
        LOGGER(UI);
    public:
        static constexpr uint32_t SlowBlinkIntervalMs = 1000;

        static constexpr uint32_t FastBlinkIntervalMs = 500;

        static constexpr uint32_t NetworkActivityBlinkMs = 50;

        static constexpr uint64_t ClearHoldTimeUs = 5000 * 1000;

        static constexpr uint64_t SetPowerSupplyThermometerAddressClickTimeoutUs = 1000 * 1000;

        static constexpr uint8_t SetPowerSupplyThermometerAddressClickCount = 10;

        explicit UserInterfaceSubsystem(mpf::core::SubsystemList * list, hw::OneWireSubsystem * oneWire);

        void initialize() override;

        void setUp() override;

        [[nodiscard]]
        bool hasWork() const override;

        void work() override;

        [[nodiscard]]
        NetworkStatusLEDState networkStatusLedState() const;

        void setNetworkStatusLedState(NetworkStatusLEDState state);

        void blinkForNetworkActivity();

    private:
        using StatusLED = hw::SystemPins::StatusLED;

        using ActivityLED = hw::SystemPins::ActivityLED;

        using ClearButton = hw::SystemPins::ClearButton;

        hw::OneWireSubsystem * const m_oneWire;

        core::Alarm m_alarm;

        core::Alarm m_clearButtonClickAlarm;

        std::atomic<NetworkStatusLEDState> m_networkStatusLedState = NetworkStatusLEDState::Off;

        std::atomic_flag m_clearButtonPressedFlag = false;

        std::atomic_uint64_t m_clearButtonPressStart = 0;

        std::atomic_uint m_setPowerSupplyThermometerAddressClicks = 0;

        [[nodiscard]]
        bool shouldTriggerClear() const;

        [[nodiscard]]
        bool shouldTriggerSetPowerSupplyThermometerAddress() const;

        void onClearButtonInterrupt(uint8_t gpio, hw::GPIOInterruptTrigger trigger);
    };

}
