/**
 * LightSubsystem.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/output/LightSubsystem.h"

#include <cassert>

#include "kilight/hw/SystemPins.h"

using mpf::core::SubsystemList;

using kilight::hw::SystemPins;
using kilight::com::WifiSubsystem;
using kilight::com::state_data_t;
using kilight::com::write_request_t;
using kilight::com::OutputIdentifier;

namespace kilight::output {
    LightSubsystem::LightSubsystem(SubsystemList* const list,
                                   WifiSubsystem* const wifiSubsystem) :
        Subsystem(list),
        m_wifi(wifiSubsystem) {
        assert(m_wifi != nullptr);
    }

    void LightSubsystem::initialize() {
        SystemPins::OutputA::enablePWM();
    }

    void LightSubsystem::setUp() {
        m_outputA.pending = output_data_t{};

        m_wifi->setWriteRequestCallback([this](write_request_t const& writeRequest) {
            if (writeRequest.outputId == OutputIdentifier::OutputA) {
                m_outputA.pending = output_data_t{writeRequest};
            }
        });

        m_outputA.onChange = [this](output_data_t const&, output_data_t const& newValue) {
            m_wifi->updateStateData([&newValue](state_data_t& state) {
                state.outputA.color = newValue.color;
                state.outputA.brightness = newValue.brightnessMultiplier;
                state.outputA.on = newValue.powerOn;
            });
        };
    }

    bool LightSubsystem::hasWork() const {
        return m_outputA.live != m_outputA.pending;
    }

    void LightSubsystem::work() {
        TRACE("Light data syncing");
        bool outputAChanged = false;

        {
            auto const lock = m_criticalSection.lock();
            outputAChanged = m_outputA.updateLive();
        }

        if (outputAChanged) {
            m_outputA.calculateTargetOutput();
            startFadeAlarm();
        }
    }

    void LightSubsystem::powerOffOutputA() {
        m_outputA.pending.powerOn = false;
    }

    bool LightSubsystem::output_state_t::updateLive() {
        if (live == pending) {
            return false;
        }
        previous = live;
        live = pending;
        return true;
    }

    void LightSubsystem::output_state_t::calculateTargetOutput() {
        if (!live.powerOn) {
            target = rgbcw_color_volatile_t { 0, 0, 0, 0, 0 };
        } else {
            target = live.getRGBCWColorScaledToBrightness();
        }
        DEBUG("Set {} = {}", name, target);
        if (onChange) {
            onChange(previous, live);
        }
    }

    void LightSubsystem::startFadeAlarm() {
        m_fadeAlarm.setTimeout(FadeTickMs, [this](core::Alarm & alarm) {
            if (m_outputA.current != m_outputA.target) {
                m_outputA.current.incrementTowards(m_outputA.target);
                writeCurrentOutput<SystemPins::OutputA>(m_outputA);
            }
            if (m_outputA.current != m_outputA.target) {
                alarm.restart(FadeTickMs);
            }
        });
    }

}
