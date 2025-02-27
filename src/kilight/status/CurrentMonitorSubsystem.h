/**
 * CurrentMonitorSubsystem.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <mpf/core/Logging.h>
#include <mpf/core/Subsystem.h>

#include "kilight/core/Alarm.h"
#include "kilight/com/WifiSubsystem.h"
#include "kilight/output/LightSubsystem.h"

namespace kilight::status {

    class CurrentMonitorSubsystem final : public mpf::core::Subsystem {
        LOGGER(CurrentMonitor);
    public:
        static constexpr uint32_t CheckCurrentEveryMs = 100;

        static constexpr uint8_t AmplifierFixedGain = 50;

        static constexpr float CurrentShuntResistorOhms = 0.0075f;

        static constexpr float MilliAmpsPerAmplifiedMilliVolt = 1 / (AmplifierFixedGain * CurrentShuntResistorOhms);

        static constexpr uint16_t OutputOverCurrentTripMilliAmps = 7900;

        CurrentMonitorSubsystem(mpf::core::SubsystemList * list,
                                com::WifiSubsystem * wifiSubsystem,
                                output::LightSubsystem * lightSubsystem);

        ~CurrentMonitorSubsystem() override = default;

        void setUp() override;

        [[nodiscard]]
        bool hasWork() const override;

        void work() override;

    private:
        static uint32_t calculateCurrent(uint32_t sampleValue);

        com::WifiSubsystem * const m_wifi;

        output::LightSubsystem * const m_lights;

        uint32_t m_outputACurrent = 0;

        #if KILIGHT_HAS_OUTPUT_B
        uint32_t m_outputBCurrent = 0;
        #endif

        core::Alarm m_alarm;

        bool volatile m_alarmFired = false;

        void processData();
    };

}
