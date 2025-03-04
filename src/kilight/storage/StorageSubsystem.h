/**
 * StorageSubsystem.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <mpf/core/Logging.h>
#include <mpf/core/Subsystem.h>

#include <kilight/protocol/OutputIdentifier.h>

#include "kilight/core/Alarm.h"
#include "kilight/storage/save_data.h"
#include "kilight/core/CriticalSection.h"

namespace kilight::storage {

    class StorageSubsystem final : public mpf::core::Subsystem {
        LOGGER(Storage);
    public:
        static constexpr uint32_t SaveCheckEveryMs = 500;

        [[nodiscard]]
        static save_data_t const & saveData();

        [[nodiscard]]
        static bool saveDataValid();

        explicit StorageSubsystem(mpf::core::SubsystemList * list);

        void initialize() override;

        void setUp() override;

        [[nodiscard]]
        bool hasWork() const override;

        void work() override;

        template<typename FuncT>
        void updatePendingData(FuncT && updateFunction) {
            updateFunction(m_pendingSaveData);
        }

        template <typename UpdateFuncT>
        void updatePendingOutputData(protocol::OutputIdentifier const outputId, UpdateFuncT&& updateFunc) {
            if (outputId == protocol::OutputIdentifier::OutputA) {
                updateFunc(m_pendingSaveData.outputA);
            } else if (outputId == protocol::OutputIdentifier::OutputB) {
                #ifdef KILIGHT_HAS_OUTPUT_B
                updateFunc(m_pendingSaveData.outputB);
                #endif
            }
        }

        [[noreturn]]
        static void clearAndReboot();


        [[noreturn]]
        void saveAndReboot();

    private:
        core::Alarm m_saveCheckAlarm;

        core::CriticalSection m_criticalSection;

        bool volatile m_saveCheckPending = false;

        save_data_t m_pendingSaveData;

        void writePendingData();
    };

}
