/**
 * OneWireSubsystem.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <array>
#include <functional>

#include <mpf/core/Logging.h>
#include <mpf/core/Subsystem.h>

#include "kilight/core/Alarm.h"
#include "kilight/hw/DS2485Driver.h"
#include "kilight/hw/DS18B20Driver.h"
#include "kilight/hw/TMP1826Driver.h"

namespace kilight::hw {

    class OneWireSubsystem final : public mpf::core::Subsystem {
        LOGGER(OneWire);

        enum class State : uint8_t {
            Invalid,
            Waiting,

            ReadMasterConfigurationStart,
            ReadMasterConfigurationComplete,
            WriteMasterConfigurationStart,
            WriteMasterConfigurationComplete,

            ReadRPUPBUFConfigurationStart,
            ReadRPUPBUFConfigurationComplete,
            WriteRPUPBUFConfigurationStart,
            WriteRPUPBUFConfigurationComplete,

            ScanForDevicesStart,
            ScanForDevicesComplete,

            ReadOnboardDeviceScratchpadCommandStart,
            ReadOnboardDeviceScratchpadCommandComplete,
            ReadOnboardDeviceScratchpadStart,
            ReadOnboardDeviceScratchpadComplete,
            WriteOnboardDeviceConfigStart,
            WriteOnboardDeviceConfigComplete,

            RequestTemperatureConversionStart,
            RequestTemperatureConversionComplete,
            ReadDeviceScratchpadCommandStart,
            ReadDeviceScratchpadCommandComplete,
            ReadDeviceScratchpadStart,
            ReadDeviceScratchpadComplete
        };
    public:
        static constexpr uint64_t ErrorRetryDelayUs = 1000 * 1000;

        static constexpr uint64_t OnboardDeviceOnlyReadDelayUs = 1000 * 1000;

        static constexpr size_t MaxExternalDevicesToFind = 2;

        explicit OneWireSubsystem(mpf::core::SubsystemList * list);

        ~OneWireSubsystem() override = default;

        void setUp() override;

        [[nodiscard]]
        bool hasWork() const override;

        void work() override;

        template<typename FuncT>
        bool registerTemperatureUpdateCallback(onewire_address_t const deviceAddress, FuncT && callback) {
            for (size_t index = 0; index < m_externalDevicesFound; ++index) {
                if (m_foundExternalDevices[index].address() == deviceAddress) {
                    m_foundExternalDevices[index].setOnTemperatureReadyCallback(std::forward<FuncT>(callback));
                    return true;
                }
            }
            return false;
        }

        template<typename FuncT>
        bool registerOnboardTemperatureUpdateCallback(FuncT && callback) {
            if (m_onboardDevice.address()) {
                m_onboardDevice.setOnTemperatureReadyCallback(std::forward<FuncT>(callback));
                return true;
            }
            return false;
        }

    private:
        State volatile m_state = State::Invalid;

        State volatile m_stateAfterWait = State::Invalid;

        DS2485Driver m_driver;

        core::Alarm m_alarm;

        DS2485Driver::master_configuration_t m_masterConfiguration {};

        DS2485Driver::rpupbuf_configuration_t m_rpupbufConfiguration {};

        size_t m_deviceAddressesFound = 0;

        std::array<onewire_address_t, MaxExternalDevicesToFind + 1> m_foundDeviceAddresses {};

        TMP1826Driver m_onboardDevice {&m_driver};

        size_t m_externalDevicesFound = 0;

        std::array<DS18B20Driver, MaxExternalDevicesToFind> m_foundExternalDevices {
            DS18B20Driver(&m_driver),
            DS18B20Driver(&m_driver)
        };

        size_t m_currentExternalDevice = 0;

        void wait(State stateAfterWait, uint64_t waitTimeUs);

        void readMasterConfigurationStartState();

        void readMasterConfigurationCompleteState();

        void writeMasterConfigurationStartState();

        void writeMasterConfigurationCompleteState();

        void readRPUPBUFConfigurationStartState();

        void readRPUPBUFConfigurationCompleteState();

        void writeRPUPBUFConfigurationStartState();

        void writeRPUPBUFConfigurationCompleteState();

        void scanForDevicesStartState();

        void scanForDevicesCompleteState();

        void readOnboardDeviceScratchpadCommandStartState();

        void readOnboardDeviceScratchpadCommandCompleteState();

        void readOnboardDeviceScratchpadStartState();

        void readOnboardDeviceScratchpadCompleteState();

        void writeOnboardDeviceConfigStartState();

        void writeOnboardDeviceConfigCompleteState();

        void readDeviceScratchpadCommandStartState();

        void readDeviceScratchpadCommandCompleteState();

        void readDeviceScratchpadStartState();

        void readDeviceScratchpadCompleteState();

        void requestTemperatureConversionStartState();

        void requestTemperatureConversionCompleteState();

    };

}
