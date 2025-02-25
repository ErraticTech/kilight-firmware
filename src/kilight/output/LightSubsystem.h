/**
 * LightSubsystem.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <mpf/core/Logging.h>
#include <mpf/core/Subsystem.h>

#include "kilight/core/Alarm.h"
#include "kilight/com/WifiSubsystem.h"
#include "kilight/core/CriticalSection.h"
#include "kilight/hw/SystemPins.h"
#include "kilight/output/output_data.h"
#include "kilight/storage/StorageSubsystem.h"

namespace kilight::output {

    class LightSubsystem final : public mpf::core::Subsystem {
        LOGGER(Light);
    public:
        static constexpr uint32_t FadeTickMs = 5;

        LightSubsystem(mpf::core::SubsystemList * list,
                       storage::StorageSubsystem * storageSubsystem,
                       com::WifiSubsystem * wifiSubsystem);

        ~LightSubsystem() override = default;

        void initialize() override;

        void setUp() override;

        [[nodiscard]]
        bool hasWork() const override;

        void work() override;

        void powerOffOutputA();

    private:

        struct output_state_t {
            std::string_view const name {};

            rgbcw_color_volatile_t target {};

            rgbcw_color_volatile_t current {};

            output_data_t live {};

            output_data_t pending {};

            output_data_t previous {};

            std::function<void(output_data_t const &, output_data_t const &)> onChange {};

            output_state_t() = delete;

            explicit output_state_t(std::string_view const & name) : name(name) {}

            bool updateLive();

            void calculateTargetOutput();
        };

        template<typename OutputPinGroupT>
        static void writeCurrentOutput(output_state_t const & outputToWrite) {
            OutputPinGroupT::Red::writePWM(outputToWrite.current.red);
            OutputPinGroupT::Green::writePWM(outputToWrite.current.green);
            OutputPinGroupT::Blue::writePWM(outputToWrite.current.blue);
            OutputPinGroupT::ColdWhite::writePWM(outputToWrite.current.coldWhite);
            OutputPinGroupT::WarmWhite::writePWM(outputToWrite.current.warmWhite);
        }

        com::WifiSubsystem * const m_wifi;

        storage::StorageSubsystem * const m_storage;

        core::CriticalSection m_criticalSection;

        core::Alarm m_fadeAlarm;

        output_state_t m_outputA { "Output A" };

        void startFadeAlarm();

        bool updateLiveOutputs();
    };
}
