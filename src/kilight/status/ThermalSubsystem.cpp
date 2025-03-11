/**
 * ThermalSubsystem.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/status/ThermalSubsystem.h"

#include <pico/time.h>

#include <kilight/protocol/SystemState.h>

#include "kilight/hw/SystemPins.h"

using kilight::protocol::SystemState;
using kilight::protocol::OutputIdentifier;
using kilight::protocol::OutputState;
using kilight::hw::SystemPins;
using kilight::hw::GPIOInterruptTrigger;
using kilight::core::Alarm;

namespace kilight::status {
    ThermalSubsystem::ThermalSubsystem(mpf::core::SubsystemList* const list,
                                       hw::OneWireSubsystem* const oneWire,
                                       com::WifiSubsystem* const wifiSubsystem,
                                       output::LightSubsystem* const lightSubsystem) :
        Subsystem(list), m_oneWire(oneWire), m_wifi(wifiSubsystem), m_lights(lightSubsystem) {
    }

    void ThermalSubsystem::initialize() {
        SystemPins::FanPWM::enablePWM(FanPWMFrequency);
        SystemPins::FanTacho::setDirection(false);
    }

    void ThermalSubsystem::setUp() {
        SystemPins::FanPWM::writePerThou(1000 - m_fanOutputPerThou);
        m_wifi->updateStateData([this](SystemState& state) {
            state.mutable_fan().set_rpm(m_fanRPM);
            state.mutable_fan().set_outputPerThou(m_fanOutputPerThou);
            state.mutable_temperatures().set_driver(m_driverTemperature);
            if (m_powerSupplyTemperature != INT16_MIN) {
                state.mutable_temperatures().set_powerSupply(m_powerSupplyTemperature);
            } else {
                state.mutable_temperatures().clear_powerSupply();
            }
        });

        m_tachometerStartUs = time_us_64();
        SystemPins::FanTacho::setInterrupt(GPIOInterruptTrigger::EdgeFalling,
                                           [this](uint8_t, GPIOInterruptTrigger) {
                                               m_tachometerCountSinceLastCheck = m_tachometerCountSinceLastCheck + 1;
                                           });

        m_state = State::PreSleep;
    }

    bool ThermalSubsystem::hasWork() const {
        return m_state != State::Sleep && m_state != State::Wait;
    }

    void ThermalSubsystem::work() {
        switch (m_state) {
            using enum State;

        case PreSleep: {
            preSleepState();
        }
        break;

        case CheckRegisterOneWireCallbacks: {
            checkRegisterOneWireCallbacksState();
        }
        break;

        case MeasureFan: {
            measureFanState();
        }
        break;

        case UpdateFanSpeed: {
            updateFanSpeedState();
        }
        break;

        case CheckOverheat: {
            checkOverheatState();
        }
        break;

        default:
            break;
        }
    }

    int16_t ThermalSubsystem::currentMaxTemp() const {
        return static_cast<int16_t>(std::max(m_driverTemperature, m_powerSupplyTemperature) / 100);
    }

    bool ThermalSubsystem::driverTempValid() const {
        return m_driverTemperature > INT16_MIN;
    }

    bool ThermalSubsystem::powerSupplyTempValid() const {
        return m_powerSupplyTemperature > INT16_MIN;
    }

    void ThermalSubsystem::preSleepState() {
        using enum State;
        m_state = Sleep;
        m_alarm.setTimeout(CheckThermalsEveryMs,
                           [this](Alarm const&) {
                               if (m_state == Sleep) {
                                   if (m_powerSupplyCallbackRegistered && m_driverCallbackRegistered) {
                                       m_state = MeasureFan;
                                   } else {
                                       m_state = CheckRegisterOneWireCallbacks;
                                   }
                               }
                           });
    }

    void ThermalSubsystem::measureFanState() {
        m_criticalSection.enter();

        uint64_t const tachometerDeltaUs = time_us_64() - m_tachometerStartUs;
        uint32_t const tachometerCount = m_tachometerCountSinceLastCheck;

        m_tachometerStartUs = time_us_64();
        m_tachometerCountSinceLastCheck = 0;

        // Exit the critical section early so we can do division without blocking everything else
        m_criticalSection.exit();

        double const tachometerDeltaMinutes = static_cast<double>(tachometerDeltaUs) / MicrosecondsPerMinute;
        m_fanRPM = static_cast<uint16_t>(static_cast<double>(tachometerCount) / FanTachometerTicksPerRevolution /
                                         tachometerDeltaMinutes);

        TRACE("Fan RPM: {} / Level: {}", m_fanRPM, m_fanOutputPerThou);

        m_wifi->updateStateData([this](SystemState& state) {
            state.mutable_fan().set_rpm(m_fanRPM);
        });

        m_state = State::UpdateFanSpeed;
    }

    void ThermalSubsystem::updateFanSpeedState() {
        calculateFanOutput();

        SystemPins::FanPWM::writePerThou(1000 - m_fanOutputPerThou);
        m_wifi->updateStateData([this](SystemState& state) {
            state.mutable_fan().set_outputPerThou(m_fanOutputPerThou);
        });

        m_state = State::CheckOverheat;
    }

    void ThermalSubsystem::checkRegisterOneWireCallbacksState() {
        if (!m_driverCallbackRegistered) {
            bool const registered = m_oneWire->registerOnboardTemperatureUpdateCallback(
                [this](int16_t const newTemperature) {
                    m_driverTemperature = newTemperature;
                    m_wifi->updateStateData([newTemperature](SystemState& state) {
                        state.mutable_temperatures().set_driver(newTemperature);
                    });
                    TRACE("Driver temperature: {:.2f} °C", static_cast<float>(m_driverTemperature) / 100);
                });
            if (registered) {
                m_driverCallbackRegistered = true;
                DEBUG("Registered driver thermometer callback");
            }
        }

        if (!m_powerSupplyCallbackRegistered) {
            bool const registered = m_oneWire->registerPowerSupplyTemperatureUpdateCallback(
                [this](int16_t const newTemperature) {
                    m_powerSupplyTemperature = newTemperature;
                    m_wifi->updateStateData([newTemperature](SystemState& state) {
                        state.mutable_temperatures().set_powerSupply(newTemperature);
                    });
                    TRACE("Power Supply temperature: {:.2f} °C", static_cast<float>(m_powerSupplyTemperature) / 100);
                });
            if (registered) {
                m_powerSupplyCallbackRegistered = true;
                DEBUG("Registered power supply thermometer callback");
            }
        }

        if (!m_outputACallbackRegistered) {
            bool const registered = m_oneWire->registerOutputATemperatureUpdateCallback(
                [this](int16_t const newTemperature) {
                    m_outputATemperature = newTemperature;
                    m_wifi->updateOutputStateData(OutputIdentifier::OutputA,
                                                  [newTemperature](OutputState& state) {
                                                      state.set_temperature(newTemperature);
                                                  });
                    TRACE("Output A temperature: {:.2f} °C", static_cast<float>(m_outputATemperature) / 100);
                });
            if (registered) {
                m_outputACallbackRegistered = true;
                DEBUG("Registered Output A thermometer callback");
            }
        }

        m_state = State::MeasureFan;
    }

    void ThermalSubsystem::checkOverheatState() {
        if (auto const maxSensedTemp = currentMaxTemp();
            maxSensedTemp >= OverheatTemperatureC) {
            WARN("Overheat trip! Current max sensed temp: {}", maxSensedTemp);
            m_lights->powerOffOutputA();
            #ifdef KILIGHT_HAS_OUTPUT_B
            m_lights->powerOffOutputB();
            #endif
        }
        m_state = State::PreSleep;
    }

    void ThermalSubsystem::calculateFanOutput() {
        if (!driverTempValid()) {
            DEBUG("Driver temperature not valid yet, setting fan output to safe default.");
            m_fanOutputPerThou = InitialFanOutputPerThou;
            return;
        }

        int16_t const maxTempReading = currentMaxTemp();
        auto const tempOverTurnOnPoint = static_cast<int16_t>(maxTempReading - FanTurnOnTemperatureC);

        if (tempOverTurnOnPoint < 0) {
            TRACE("Fan stopped for low temp: {}", maxTempReading);
            m_fanOutputPerThou = 0;
            return;
        }

        if (tempOverTurnOnPoint >= FanAdjustTemperatureRangeC) {
            TRACE("Fan adjusting to max for temp {}", maxTempReading);
            m_fanOutputPerThou = MaxFanOutputPerThou;
            return;
        }

        float const fractionOfRange = static_cast<float>(tempOverTurnOnPoint) / FanAdjustTemperatureRangeC;
        m_fanOutputPerThou = MinFanOutputPerThou + static_cast<uint16_t>(FanOutputRangePerThou * fractionOfRange);
        TRACE("Fan adjusting for temp {} to power {}", maxTempReading, m_fanOutputPerThou);
    }

    void ThermalSubsystem::wait(uint32_t const waitTimeMs, State const stateAfterWaiting) {
        m_state = State::Wait;
        m_stateAfterWait = stateAfterWaiting;
        m_alarm.setTimeout(waitTimeMs,
                           [this](Alarm const&) {
                               if (m_state == State::Wait) {
                                   m_state = m_stateAfterWait;
                               }
                           });
    }
}
