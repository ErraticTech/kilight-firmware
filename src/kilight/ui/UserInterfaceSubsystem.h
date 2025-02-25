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
#include "kilight/storage/StorageSubsystem.h"

namespace kilight::ui {

    class UserInterfaceSubsystem final : public mpf::core::Subsystem {
        LOGGER(UI);
    public:
        static constexpr uint32_t SlowBlinkIntervalMs = 1000;

        static constexpr uint32_t FastBlinkIntervalMs = 500;

        static constexpr uint32_t NetworkActivityBlinkMs = 50;

        static constexpr uint64_t ClearHoldTimeUs = 5000 * 1000;

        enum class NetworkStatusLEDState : uint8_t {
            Off = 0,
            Searching = 1,
            Connecting = 2,
            Connected = 3
        };

        explicit UserInterfaceSubsystem(mpf::core::SubsystemList * list);

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

        core::Alarm m_alarm;

        NetworkStatusLEDState volatile m_networkStatusLedState = NetworkStatusLEDState::Off;

        bool volatile m_clearButtonPressed = false;

        uint64_t volatile m_clearButtonPressStart = 0;

        [[nodiscard]]
        bool shouldTriggerClear() const;
    };

}
