/**
 * StorageSubsystem.cpp
 *
 * @author Patrick Lavigne
 */
#include "kilight/storage/StorageSubsystem.h"

#include <bit>
#include <cstring>
#include <array>
#include <boot/picoboot_constants.h>

#include <pico/flash.h>
#include <hardware/flash.h>
#include <pico/bootrom.h>

#include "kilight/conf/WifiConfig.h"
#include "kilight/util/MathUtil.h"

using std::array;
using mpf::core::FixedPackedString;

using kilight::conf::getWifiConfig;
using kilight::core::Alarm;
using kilight::util::MathUtil;

namespace kilight::storage {

    static constexpr size_t SaveDataTotalAvailableSize = FLASH_SECTOR_SIZE;

    static constexpr size_t SaveDataWriteSize = FLASH_PAGE_SIZE;

    static constexpr uintptr_t SaveDataFlashOffset = PICO_FLASH_SIZE_BYTES - SaveDataTotalAvailableSize;

    struct PACKED save_data_wrapper_t {
        save_data_t data = {};
        uint16_t volatile crc = 0;
        std::array<uint8_t, SaveDataWriteSize - sizeof(save_data_t) - sizeof(uint16_t)> padding {};
    };

    static auto const SaveDataPtr = std::bit_cast<save_data_wrapper_t const *>(XIP_BASE + SaveDataFlashOffset);

    static_assert(sizeof(save_data_wrapper_t) <= SaveDataTotalAvailableSize,
                  "save_data_wrapper_t is larger than available save data size.");

    static_assert(sizeof(save_data_wrapper_t) <= SaveDataWriteSize,
                  "save_data_wrapper_t is larger than available save data write size.");

    save_data_t const& StorageSubsystem::saveData() {
        return SaveDataPtr->data;
    }

    bool StorageSubsystem::saveDataValid() {
        uint16_t const calculatedCRC = MathUtil::crc16(SaveDataPtr->data);
        return SaveDataPtr->crc == calculatedCRC;
    }

    StorageSubsystem::StorageSubsystem(mpf::core::SubsystemList* const list) :
        Subsystem(list) {
    }

    void StorageSubsystem::initialize() {
        if (!saveDataValid()) {
            INFO("No valid save data found, initializing with defaults");
            m_pendingSaveData = save_data_t{
                .wifi = com::wifi_data_t(getWifiConfig()),
                .outputA = output::output_data_t{}
            };
            writePendingData();
        } else {
            m_pendingSaveData = saveData();
            DEBUG("Valid save data found");
        }
    }

    void StorageSubsystem::setUp() {
        m_saveCheckAlarm.setTimeout(SaveCheckEveryMs,
                                    [this](Alarm const&) {
                                        m_saveCheckPending = true;
                                    });
    }

    bool StorageSubsystem::hasWork() const {
        return m_saveCheckPending;
    }

    void StorageSubsystem::work() {
        m_saveCheckPending = false;

        if (saveData() != m_pendingSaveData) {
            DEBUG("Pending data changed, saving...");
            writePendingData();
        }

        m_saveCheckAlarm.setTimeout(SaveCheckEveryMs,
                                    [this](Alarm const&) {
                                        m_saveCheckPending = true;
                                    });
    }

    void StorageSubsystem::clearAndReboot() {
        flash_safe_execute([](void *) {
                               flash_range_erase(SaveDataFlashOffset, SaveDataTotalAvailableSize);
                           },
                           nullptr,
                           UINT32_MAX);
        INFO("Cleared storage. Rebooting...");
        rom_reboot(REBOOT2_FLAG_REBOOT_TYPE_NORMAL | REBOOT2_FLAG_NO_RETURN_ON_SUCCESS, 1, 0, 0);
        __builtin_unreachable();
    }

    void StorageSubsystem::writePendingData() {
        save_data_wrapper_t dataToWrite;
        m_criticalSection.enter();
        dataToWrite.data = m_pendingSaveData;
        m_criticalSection.exit();
        dataToWrite.crc = MathUtil::crc16(dataToWrite.data);

        auto buff = std::bit_cast<array<uint8_t, SaveDataWriteSize>>(dataToWrite);

        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        flash_safe_execute([](void * context) {
                               auto const innerBuff = static_cast<array<uint8_t, SaveDataWriteSize> const*>(context);
                               flash_range_erase(SaveDataFlashOffset, SaveDataTotalAvailableSize);
                               flash_range_program(SaveDataFlashOffset, innerBuff->data(), SaveDataWriteSize);
                           },
                           &buff,
                           UINT32_MAX);
    }
}
