/**
 * ThermalSubsystem.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <mpf/core/Logging.h>
#include <mpf/core/Subsystem.h>

#include "kilight/core/Alarm.h"
#include "kilight/core/CriticalSection.h"
#include "kilight/com/WifiSubsystem.h"
#include "kilight/hw/OneWireSubsystem.h"
#include "kilight/output/LightSubsystem.h"

namespace kilight::status {

    class ThermalSubsystem final : public mpf::core::Subsystem {
        LOGGER(Thermal);

        enum class State : uint8_t {
            Invalid,
            PreSleep,
            Sleep,
            Wait,
            CheckRegisterOneWireCallbacks,
            MeasureFan,
            UpdateFanSpeed,
            CheckOverheat
        };
    public:
        static constexpr int16_t FanTurnOnTemperatureC = 25;

        static constexpr int16_t FanMaxTemperatureC = 60;

        static constexpr int16_t FanAdjustTemperatureRangeC = FanMaxTemperatureC - FanTurnOnTemperatureC;

        static constexpr int16_t OverheatTemperatureC = 80;

        static constexpr uint32_t CheckThermalsEveryMs = 1000;

        static constexpr uint32_t OnBoardThermometerMeasurementWaitTimeMs = 10;

        static constexpr uint32_t FanPWMFrequency = 25000;

        static constexpr uint16_t MinFanOutputPerThou = 100;

        static constexpr uint16_t MaxFanOutputPerThou = 1000;

        static constexpr uint16_t FanOutputRangePerThou = MaxFanOutputPerThou - MinFanOutputPerThou;

        static constexpr uint16_t InitialFanOutputPerThou = 800;

        static constexpr uint8_t FanTachometerTicksPerRevolution = 2;

        static constexpr uint32_t MicrosecondsPerMinute = 1000 * 1000 * 60;

        ThermalSubsystem(mpf::core::SubsystemList * list,
                         hw::OneWireSubsystem * oneWire,
                         com::WifiSubsystem * wifiSubsystem,
                         output::LightSubsystem *lightSubsystem);

        ~ThermalSubsystem() override = default;

        void initialize() override;

        void setUp() override;

        [[nodiscard]]
        bool hasWork() const override;

        void work() override;

        [[nodiscard]]
        int16_t currentMaxTemp() const;

        [[nodiscard]]
        bool driverTempValid() const;

        [[nodiscard]]
        bool powerSupplyTempValid() const;

    private:
        State volatile m_state = State::Invalid;

        State m_stateAfterWait = State::Invalid;

        core::CriticalSection m_criticalSection;

        hw::OneWireSubsystem * const m_oneWire;

        com::WifiSubsystem * const m_wifi;

        output::LightSubsystem * const m_lights;

        core::Alarm m_alarm;

        uint32_t volatile m_tachometerCountSinceLastCheck = 0;

        uint64_t m_tachometerStartUs = 0;

        uint16_t m_fanRPM = 0;

        uint16_t m_fanOutputPerThou = InitialFanOutputPerThou;

        int16_t volatile m_driverTemperature = INT16_MIN;

        int16_t volatile m_powerSupplyTemperature = INT16_MIN;

        bool m_driverCallbackRegistered = false;

        bool m_powerSupplyCallbackRegistered = false;

        void preSleepState();

        void measureFanState();

        void updateFanSpeedState();

        void checkRegisterOneWireCallbacksState();

        void checkOverheatState();

        void calculateFanOutput();

        void wait(uint32_t waitTimeMs, State stateAfterWaiting);

    };

}
