/**
 * UserInterfaceSubsystem.cpp
 *
 * @author Patrick Lavigne
 */
#include "kilight/ui/UserInterfaceSubsystem.h"

#include <functional>
#include <cassert>

using kilight::hw::GPIOInterruptTrigger;
using kilight::storage::StorageSubsystem;
using kilight::core::Alarm;

namespace kilight::ui {
    UserInterfaceSubsystem::UserInterfaceSubsystem(mpf::core::SubsystemList* const list,
                                                   hw::OneWireSubsystem* const oneWire) :
        Subsystem(list),
        m_oneWire(oneWire) {
        assert(m_oneWire != nullptr);
    }

    void UserInterfaceSubsystem::initialize() {
        StatusLED::setOutput();
        ActivityLED::setOutput();
        ClearButton::setInput();
        ClearButton::setPullUp();
    }

    void UserInterfaceSubsystem::setUp() {
        if (!ClearButton::read()) {
            m_clearButtonPressStart = time_us_64();
            m_clearButtonPressedFlag.test_and_set();
        }
        ClearButton::setInterrupt(GPIOInterruptTrigger::EdgeFalling | GPIOInterruptTrigger::EdgeRising,
                                  std::bind_front(&UserInterfaceSubsystem::onClearButtonInterrupt, this));
    }

    bool UserInterfaceSubsystem::hasWork() const {
        return shouldTriggerClear() || shouldTriggerSetPowerSupplyThermometerAddress();
    }

    void UserInterfaceSubsystem::work() {
        if (shouldTriggerClear()) {
            DEBUG("Clear button held, initiating settings clear...");
            StorageSubsystem::clearAndReboot();
        }
        if (shouldTriggerSetPowerSupplyThermometerAddress()) {
            DEBUG("Clear button clicked {} times, attempting to register power supply thermometer address...",
                  SetPowerSupplyThermometerAddressClickCount);
            m_clearButtonClickAlarm.cancel();
            m_setPowerSupplyThermometerAddressClicks = 0;
            m_oneWire->requestSetPowerSupplyThermometerAddress();
        }
    }

    NetworkStatusLEDState UserInterfaceSubsystem::networkStatusLedState() const {
        return m_networkStatusLedState;
    }

    void UserInterfaceSubsystem::setNetworkStatusLedState(NetworkStatusLEDState const state) {
        if (m_networkStatusLedState == state) {
            return;
        }
        m_networkStatusLedState = state;
        switch (m_networkStatusLedState) {
            using enum NetworkStatusLEDState;

        case Searching:
            m_alarm.setTimeout(SlowBlinkIntervalMs,
                               [](Alarm& alarm) {
                                   StatusLED::toggle();
                                   ActivityLED::write(false);
                                   alarm.restart(SlowBlinkIntervalMs);
                               });
            break;

        case Connecting:
            m_alarm.setTimeout(FastBlinkIntervalMs,
                               [](Alarm& alarm) {
                                   StatusLED::toggle();
                                   ActivityLED::write(false);
                                   alarm.restart(FastBlinkIntervalMs);
                               });
            break;

        case Connected:
            m_alarm.cancel();
            StatusLED::write(false);
            ActivityLED::write(true);
            break;

        default:
            m_alarm.cancel();
            StatusLED::write(false);
            ActivityLED::write(false);
            break;

        }
    }

    void UserInterfaceSubsystem::blinkForNetworkActivity() {
        if (m_networkStatusLedState != NetworkStatusLEDState::Connected) {
            return;
        }

        ActivityLED::write(false);
        m_alarm.setTimeout(NetworkActivityBlinkMs,
                           [this](Alarm const&) {
                               if (m_networkStatusLedState == NetworkStatusLEDState::Connected) {
                                   ActivityLED::write(true);
                               }
                           });
    }

    bool UserInterfaceSubsystem::shouldTriggerClear() const {
        return m_clearButtonPressedFlag.test() && time_us_64() - m_clearButtonPressStart >= ClearHoldTimeUs;
    }

    bool UserInterfaceSubsystem::shouldTriggerSetPowerSupplyThermometerAddress() const {
        return m_setPowerSupplyThermometerAddressClicks >= SetPowerSupplyThermometerAddressClickCount;
    }

    void UserInterfaceSubsystem::onClearButtonInterrupt(uint8_t, GPIOInterruptTrigger const trigger) {
        if (trigger == GPIOInterruptTrigger::EdgeFalling) {
            if (!m_clearButtonPressedFlag.test_and_set()) {
                m_clearButtonPressStart = time_us_64();
            }
        } else if (trigger == GPIOInterruptTrigger::EdgeRising) {
            m_clearButtonClickAlarm.setTimeoutUs(SetPowerSupplyThermometerAddressClickTimeoutUs,
                                                 [this](Alarm const &) {
                                                     m_setPowerSupplyThermometerAddressClicks = 0;
                                                 });
            ++m_setPowerSupplyThermometerAddressClicks;
            m_clearButtonPressedFlag.clear();
        }
    }
}
