/**
 * UserInterfaceSubsystem.cpp
 *
 * @author Patrick Lavigne
 */
#include "kilight/ui/UserInterfaceSubsystem.h"

#include "kilight/hw/SystemPins.h"

using kilight::hw::SystemPins;
using kilight::hw::GPIOInterruptTrigger;
using kilight::storage::StorageSubsystem;

namespace kilight::ui {
    UserInterfaceSubsystem::UserInterfaceSubsystem(mpf::core::SubsystemList* const list) :
        Subsystem(list) {
    }

    void UserInterfaceSubsystem::initialize() {
        SystemPins::StatusLED::setOutput();
        SystemPins::ActivityLED::setOutput();
        SystemPins::ClearButton::setInput();
        SystemPins::ClearButton::setPullUp();
    }

    void UserInterfaceSubsystem::setUp() {
        using enum GPIOInterruptTrigger;
        if (!SystemPins::ClearButton::read()) {
            m_clearButtonPressStart = time_us_64();
            m_clearButtonPressed = true;
        }
        SystemPins::ClearButton::setInterrupt(EdgeFalling | EdgeRising,
                                              [this](uint8_t, GPIOInterruptTrigger const trigger) {
                                                  if (!m_clearButtonPressed && trigger == EdgeFalling) {
                                                      m_clearButtonPressStart = time_us_64();
                                                      m_clearButtonPressed = true;
                                                  } else {
                                                      m_clearButtonPressed = false;
                                                  }
                                              });
    }

    bool UserInterfaceSubsystem::hasWork() const {
        return shouldTriggerClear();
    }

    void UserInterfaceSubsystem::work() {
        if (shouldTriggerClear()) {
            DEBUG("Clear button held, initiating settings clear...");
            StorageSubsystem::clearAndReboot();
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
                               [](core::Alarm& alarm) {
                                   SystemPins::StatusLED::toggle();
                                   SystemPins::ActivityLED::write(false);
                                   alarm.restart(SlowBlinkIntervalMs);
                               });
            break;

        case Connecting:
            m_alarm.setTimeout(FastBlinkIntervalMs,
                               [](core::Alarm& alarm) {
                                   SystemPins::StatusLED::toggle();
                                   SystemPins::ActivityLED::write(false);
                                   alarm.restart(FastBlinkIntervalMs);
                               });
            break;

        case Connected:
            m_alarm.cancel();
            SystemPins::StatusLED::write(false);
            SystemPins::ActivityLED::write(true);
            break;

        default:
            m_alarm.cancel();
            SystemPins::StatusLED::write(false);
            SystemPins::ActivityLED::write(false);
            break;

        }
    }

    void UserInterfaceSubsystem::blinkForNetworkActivity() {
        if (m_networkStatusLedState != NetworkStatusLEDState::Connected) {
            return;
        }

        SystemPins::ActivityLED::write(false);
        m_alarm.setTimeout(NetworkActivityBlinkMs,
                           [this](core::Alarm const&) {
                               if (m_networkStatusLedState == NetworkStatusLEDState::Connected) {
                                   SystemPins::ActivityLED::write(true);
                               }
                           });
    }

    bool UserInterfaceSubsystem::shouldTriggerClear() const {
        return m_clearButtonPressed && time_us_64() - m_clearButtonPressStart >= ClearHoldTimeUs;
    }
}
