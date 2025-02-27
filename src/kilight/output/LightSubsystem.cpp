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
using kilight::protocol::OutputState;
using kilight::protocol::WriteOutput;
using kilight::protocol::OutputIdentifier;

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
        #ifdef KILIGHT_HAS_OUTPUT_B
        SystemPins::OutputB::enablePWM();
        #endif
    }

    void LightSubsystem::setUp() {
        m_outputA.pending = StorageSubsystem::saveData().outputA;
        #ifdef KILIGHT_HAS_OUTPUT_B
        m_outputB.pending = StorageSubsystem::saveData().outputB;
        #endif

        m_wifi->setWriteRequestCallback([this](WriteOutput const& writeRequest) {
            CommandResult response;
            switch (writeRequest.get_outputId()) {
            case OutputIdentifier::OutputA:
                m_outputA = writeRequest;
                response.set_result(CommandResult::Result::OK);
                break;

            #ifdef KILIGHT_HAS_OUTPUT_B
            case OutputIdentifier::OutputB:
                m_outputB = writeRequest;
                response.set_result(CommandResult::Result::OK);
                break;
            #endif

            default:
                response.set_result(CommandResult::Result::Error);
            }
            return response;
        });
    }

    bool LightSubsystem::hasWork() const {
        #ifdef KILIGHT_HAS_OUTPUT_B
        return m_outputA.live != m_outputA.pending || m_outputB.live != m_outputB.pending;
        #else
        return m_outputA.live != m_outputA.pending;
        #endif
    }

    void LightSubsystem::work() {
        TRACE("Light data syncing");
        if (updateLiveOutputs()) {
            m_outputA.calculateTargetOutput(*this);
            #ifdef KILIGHT_HAS_OUTPUT_B
            m_outputB.calculateTargetOutput(*this);
            #endif
            startFadeAlarm();
        }
    }

    void LightSubsystem::powerOffOutputA() {
        m_outputA.pending.powerOn = false;
    }

    #ifdef KILIGHT_HAS_OUTPUT_B
    void LightSubsystem::powerOffOutputB() {
        m_outputB.pending.powerOn = false;
    }
    #endif

    LightSubsystem::output_state_t& LightSubsystem::output_state_t::operator=(WriteOutput const& protocolWrite) {
        pending.color = protocolWrite.color();
        pending.brightnessMultiplier = static_cast<uint8_t>(protocolWrite.brightness());
        pending.powerOn = protocolWrite.on();
        return *this;
    }

    bool LightSubsystem::output_state_t::updateLive() {
        if (live == pending) {
            return false;
        }
        previous = live;
        live = pending;
        return true;
    }

    void LightSubsystem::output_state_t::calculateTargetOutput(LightSubsystem const & parent) {
        if (!live.powerOn) {
            target = rgbcw_color_volatile_t { 0, 0, 0, 0, 0 };
        } else {
            target = live.getRGBCWColorScaledToBrightness();
        }
        DEBUG("Set {} = {}", outputId == OutputIdentifier::OutputA ? "Output A" : "Output B", target);

        parent.onOutputChange(outputId, live);
    }

    void LightSubsystem::startFadeAlarm() {
        m_fadeAlarm.setTimeout(FadeTickMs, [this](core::Alarm & alarm) {
            if (m_outputA.current != m_outputA.target) {
                m_outputA.current.incrementTowards(m_outputA.target);
                writeCurrentOutput<SystemPins::OutputA>(m_outputA);
            }
            #ifdef KILIGHT_HAS_OUTPUT_B
            if (m_outputB.current != m_outputB.target) {
                m_outputB.current.incrementTowards(m_outputB.target);
                writeCurrentOutput<SystemPins::OutputB>(m_outputB);
            }
            if (m_outputA.current != m_outputA.target || m_outputB.current != m_outputB.target) {
                alarm.restart(FadeTickMs);
            }
            #else
            if (m_outputA.current != m_outputA.target) {
                alarm.restart(FadeTickMs);
            }
            #endif
        });
    }

    bool LightSubsystem::updateLiveOutputs() {
        auto const lock = m_criticalSection.lock();
        return m_outputA.updateLive();
    }

    void LightSubsystem::onOutputChange(OutputIdentifier const outputId, output_data_t const& newValue) const {
        m_wifi->updateOutputStateData(outputId, [&newValue](OutputState& output) {
            output.set_color(newValue.color.toColor());
            output.set_brightness(newValue.brightnessMultiplier);
            output.set_on(static_cast<bool>(newValue.powerOn));
        });

        m_storage->updatePendingOutputData(outputId, [&newValue](output_data_t & saveData) {
            saveData = newValue;
        });
    }

}
