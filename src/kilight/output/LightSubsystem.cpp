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
using kilight::storage::StorageSubsystem;
using kilight::storage::save_data_t;
using kilight::protocol::CommandResult;
using kilight::protocol::SystemState;
using kilight::protocol::WriteOutput;

namespace kilight::output {
    LightSubsystem::LightSubsystem(SubsystemList* const list,
                                   StorageSubsystem * const storageSubsystem,
                                   WifiSubsystem* const wifiSubsystem) :
        Subsystem(list),
        m_wifi(wifiSubsystem),
        m_storage(storageSubsystem) {
        assert(m_wifi != nullptr);
        assert(m_storage != nullptr);
    }

    void LightSubsystem::initialize() {
        SystemPins::OutputA::enablePWM();
    }

    void LightSubsystem::setUp() {
        m_outputA.pending = StorageSubsystem::saveData().outputA;

        m_wifi->setWriteRequestCallback([this](protocol::WriteOutput const& writeRequest) {
            CommandResult response;
            if (writeRequest.get_outputId() == protocol::OutputIdentifier::OutputA) {
                m_outputA.pending.color.red = static_cast<uint8_t>(writeRequest.color().red());
                m_outputA.pending.color.green = static_cast<uint8_t>(writeRequest.color().green());
                m_outputA.pending.color.blue = static_cast<uint8_t>(writeRequest.color().blue());
                m_outputA.pending.color.coldWhite = static_cast<uint8_t>(writeRequest.color().coldWhite());
                m_outputA.pending.color.warmWhite = static_cast<uint8_t>(writeRequest.color().warmWhite());
                m_outputA.pending.brightnessMultiplier = static_cast<uint8_t>(writeRequest.brightness());
                m_outputA.pending.powerOn = writeRequest.on();
                response.set_result(CommandResult::Result::OK);
            } else {
                response.set_result(CommandResult::Result::Error);
            }
            return response;
        });

        m_outputA.onChange = [this](output_data_t const&, output_data_t const& newValue) {
            m_wifi->updateStateData([&newValue](SystemState& state) {
                auto & output = state.mutable_outputA();
                output.set_color(newValue.color.toColor());
                output.set_brightness(newValue.brightnessMultiplier);
                output.set_on(static_cast<bool>(newValue.powerOn));
            });

            m_storage->updatePendingData([&newValue](save_data_t & saveData) {
                saveData.outputA = newValue;
            });
        };
    }

    bool LightSubsystem::hasWork() const {
        return m_outputA.live != m_outputA.pending;
    }

    void LightSubsystem::work() {
        TRACE("Light data syncing");
        if (updateLiveOutputs()) {
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

    bool LightSubsystem::updateLiveOutputs() {
        auto const lock = m_criticalSection.lock();
        return m_outputA.updateLive();
    }

}
