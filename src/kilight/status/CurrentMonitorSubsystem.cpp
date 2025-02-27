/**
 * CurrentMonitorSubsystem.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/status/CurrentMonitorSubsystem.h"

#include <cmath>

#include <kilight/protocol/SystemState.h>

#include "kilight/hw/ADC.h"

using kilight::hw::ADC;
using kilight::core::Alarm;

namespace kilight::status {
    CurrentMonitorSubsystem::CurrentMonitorSubsystem(mpf::core::SubsystemList* const list,
                                                     com::WifiSubsystem* const wifiSubsystem,
                                                     output::LightSubsystem* const lightSubsystem) :
        Subsystem(list),
        m_wifi(wifiSubsystem),
        m_lights(lightSubsystem) {
    }

    void CurrentMonitorSubsystem::setUp() {
        m_alarm.setTimeout(CheckCurrentEveryMs,
                           [this](Alarm const&) {
                               m_alarmFired = true;
                           });
    }

    bool CurrentMonitorSubsystem::hasWork() const {
        return m_alarmFired && (ADC::dataReady() || !ADC::conversionInProgress());
    }

    void CurrentMonitorSubsystem::work() {
        if (ADC::dataReady()) {
            processData();
        }

        if (!ADC::conversionInProgress()) {
            ADC::start();
            TRACE("Starting current measurement");
        }
    }

    uint32_t CurrentMonitorSubsystem::calculateCurrent(uint32_t const sampleValue) {
        return static_cast<uint32_t>(std::round(static_cast<float>(sampleValue) *
                                                ADC::MilliVoltsPerADCBit *
                                                MilliAmpsPerAmplifiedMilliVolt));

    }

    void CurrentMonitorSubsystem::processData() {
        uint32_t outputASum = 0;
        uint32_t outputACount = 0;

        #ifdef KILIGHT_HAS_OUTPUT_B

        uint32_t outputBSum = 0;
        uint32_t outputBCount = 0;

        for (auto const [channelZero, channelOne] : ADC::data()) {
            if (!channelZero.error) {
                outputASum += channelZero.value;
                ++outputACount;
            }
            if (!channelOne.error) {
                outputBSum += channelOne.value;
                ++outputBCount;
            }
        }

        if (outputACount > 0) {
            m_outputACurrent = calculateCurrent(outputASum / outputACount);
        }
        if (outputBCount > 0) {
            m_outputBCurrent = calculateCurrent(outputBSum / outputBCount);
        }
        TRACE("Output A: {}mA / Output B: {}mA", m_outputACurrent, m_outputBCurrent);

        #else

        for (auto const [channelZero] : ADC::data()) {
            if (!channelZero.error) {
                outputASum += channelZero.value;
                ++outputACount;
            }
        }

        if (outputACount > 0) {
            m_outputACurrent = calculateCurrent(outputASum / outputACount);
        }
        TRACE("Output A: {}mA", m_outputACurrent);

        #endif


        m_wifi->updateStateData([this](protocol::SystemState& state) {
            state.mutable_outputA().set_current(m_outputACurrent);

            #ifdef KILIGHT_HAS_OUTPUT_B
            state.mutable_outputB().set_current(m_outputBCurrent);
            #endif
        });
        ADC::clearDataReady();

        if (m_outputACurrent >= OutputOverCurrentTripMilliAmps) {
            WARN("Output A overcurrent trip! Output A Current: {}", m_outputACurrent);
            m_lights->powerOffOutputA();
        }

        #ifdef KILIGHT_HAS_OUTPUT_B

        if (m_outputBCurrent >= OutputOverCurrentTripMilliAmps) {
            WARN("Output B overcurrent trip! Output B Current: {}", m_outputBCurrent);
            m_lights->powerOffOutputB();
        }

        #endif

        m_alarmFired = false;
        m_alarm.setTimeout(CheckCurrentEveryMs,
                           [this](Alarm const&) {
                               m_alarmFired = true;
                           });
    }
}
