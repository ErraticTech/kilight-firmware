/**
 * OneWireSubsystem.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/OneWireSubsystem.h"

namespace kilight::hw {
    OneWireSubsystem::OneWireSubsystem(mpf::core::SubsystemList* const list) :
        Subsystem(list) {
    }

    void OneWireSubsystem::setUp() {
        m_state = State::ReadMasterConfigurationStart;
    }

    bool OneWireSubsystem::hasWork() const {
        return m_state != State::Waiting && m_state != State::Invalid;
    }

    void OneWireSubsystem::work() {
        switch (m_state) {
            using enum State;
        case ReadMasterConfigurationStart:
            readMasterConfigurationStartState();
            break;

        case ReadMasterConfigurationComplete:
            readMasterConfigurationCompleteState();
            break;

        case WriteMasterConfigurationStart:
            writeMasterConfigurationStartState();
            break;

        case WriteMasterConfigurationComplete:
            writeMasterConfigurationCompleteState();
            break;

        case ReadRPUPBUFConfigurationStart:
            readRPUPBUFConfigurationStartState();
            break;

        case ReadRPUPBUFConfigurationComplete:
            readRPUPBUFConfigurationCompleteState();
            break;

        case WriteRPUPBUFConfigurationStart:
            writeRPUPBUFConfigurationStartState();
            break;

        case WriteRPUPBUFConfigurationComplete:
            writeRPUPBUFConfigurationCompleteState();
            break;

        case ScanForDevicesStart:
            scanForDevicesStartState();
            break;

        case ScanForDevicesComplete:
            scanForDevicesCompleteState();
            break;

        case ReadOnboardDeviceScratchpadCommandStart:
            readOnboardDeviceScratchpadCommandStartState();
            break;

        case ReadOnboardDeviceScratchpadCommandComplete:
            readOnboardDeviceScratchpadCommandCompleteState();
            break;

        case ReadOnboardDeviceScratchpadStart:
            readOnboardDeviceScratchpadStartState();
            break;

        case ReadOnboardDeviceScratchpadComplete:
            readOnboardDeviceScratchpadCompleteState();
            break;

        case WriteOnboardDeviceConfigStart:
            writeOnboardDeviceConfigStartState();
            break;

        case WriteOnboardDeviceConfigComplete:
            writeOnboardDeviceConfigCompleteState();
            break;

        case RequestTemperatureConversionStart:
            requestTemperatureConversionStartState();
            break;

        case RequestTemperatureConversionComplete:
            requestTemperatureConversionCompleteState();
            break;

        case ReadDeviceScratchpadCommandStart:
            readDeviceScratchpadCommandStartState();
            break;

        case ReadDeviceScratchpadCommandComplete:
            readDeviceScratchpadCommandCompleteState();
            break;

        case ReadDeviceScratchpadStart:
            readDeviceScratchpadStartState();
            break;

        case ReadDeviceScratchpadComplete:
            readDeviceScratchpadCompleteState();
            break;

        default:
            break;
        }
    }

    void OneWireSubsystem::wait(State const stateAfterWait, uint64_t const waitTimeUs) {
        m_state = State::Waiting;
        m_stateAfterWait = stateAfterWait;
        m_alarm.setTimeoutUs(waitTimeUs,
                             [this](core::Alarm const&) {
                                 if (m_state == State::Waiting) {
                                     m_state = m_stateAfterWait;
                                 }
                             });
    }

    void OneWireSubsystem::readMasterConfigurationStartState() {
        if (!m_driver.startReadMasterConfiguration()) {
            wait(State::ReadMasterConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadMasterConfigurationComplete, DS2485Driver::OperationDelayTimeUs);
    }

    void OneWireSubsystem::readMasterConfigurationCompleteState() {
        if (!m_driver.completeReadMasterConfiguration(&m_masterConfiguration)) {
            wait(State::ReadMasterConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        TRACE("Master config - activePullUp: {} strongPullUp: {} powerDown: {} oneWireOverdriveSpeed: {}",
              static_cast<bool>(m_masterConfiguration.activePullUp),
              static_cast<bool>(m_masterConfiguration.strongPullUp),
              static_cast<bool>(m_masterConfiguration.powerDown),
              static_cast<bool>(m_masterConfiguration.oneWireOverdriveSpeed));

        m_state = State::WriteMasterConfigurationStart;
    }

    void OneWireSubsystem::writeMasterConfigurationStartState() {
        m_masterConfiguration.powerDown = false;
        m_masterConfiguration.activePullUp = true;
        m_masterConfiguration.strongPullUp = true;
        m_masterConfiguration.oneWireOverdriveSpeed = false;


        if (!m_driver.startWriteMasterConfiguration(m_masterConfiguration)) {
            wait(State::WriteMasterConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::WriteMasterConfigurationComplete, DS2485Driver::OperationDelayTimeUs);
    }

    void OneWireSubsystem::writeMasterConfigurationCompleteState() {
        if (!m_driver.completeWriteMasterConfiguration()) {
            wait(State::WriteMasterConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        m_state = State::ReadRPUPBUFConfigurationStart;
    }

    void OneWireSubsystem::readRPUPBUFConfigurationStartState() {
        if (!m_driver.startReadRPUPBUFConfiguration()) {
            wait(State::ReadRPUPBUFConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadRPUPBUFConfigurationComplete, DS2485Driver::OperationDelayTimeUs);
    }

    void OneWireSubsystem::readRPUPBUFConfigurationCompleteState() {
        if (!m_driver.completeReadRPUPBUFConfiguration(&m_rpupbufConfiguration)) {
            wait(State::ReadRPUPBUFConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        m_state = State::WriteRPUPBUFConfigurationStart;
    }

    void OneWireSubsystem::writeRPUPBUFConfigurationStartState() {
        m_rpupbufConfiguration.custom = true;
        m_rpupbufConfiguration.rwpu = DS2485Driver::OneWireRWPU::OneThousand;
        m_rpupbufConfiguration.viapo = DS2485Driver::OneWireVIAPO::Low;
        m_rpupbufConfiguration.vth = DS2485Driver::OneWireVTH::Medium;


        if (!m_driver.startWriteRPUPBUFConfiguration(m_rpupbufConfiguration)) {
            wait(State::WriteRPUPBUFConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::WriteRPUPBUFConfigurationComplete, DS2485Driver::OperationDelayTimeUs);
    }

    void OneWireSubsystem::writeRPUPBUFConfigurationCompleteState() {
        if (!m_driver.completeWriteRPUPBUFConfiguration()) {
            wait(State::WriteRPUPBUFConfigurationStart, ErrorRetryDelayUs);
            return;
        }

        m_state = State::ScanForDevicesStart;
    }

    void OneWireSubsystem::scanForDevicesStartState() {
        // ReSharper disable once CppDFAConstantConditions
        bool const isFirstRun = m_deviceAddressesFound == 0;
        if (isFirstRun) {
            DEBUG("Starting OneWire bus search...");
        } else {
            DEBUG("Continuing OneWire bus search...");
        }
        if (!m_driver.startOneWireSearch({
                .reset = true,
                .ignorePresencePulse = false,
                .resetSearch = isFirstRun,
                .searchCommandCode = 0xF0
            })) {
            wait(State::ScanForDevicesStart, ErrorRetryDelayUs);
            return;
        }
        wait(State::ScanForDevicesComplete, DS2485Driver::SearchDelayTimeUs);
    }

    void OneWireSubsystem::scanForDevicesCompleteState() {
        using enum State;
        DS2485Driver::onewire_search_result_t result{};
        if (!m_driver.completeOneWireSearch(&result)) {
            wait(ScanForDevicesStart, ErrorRetryDelayUs);
            return;
        }
        onewire_address_t const foundAddress = result.address;
        m_foundDeviceAddresses[m_deviceAddressesFound] = foundAddress;
        ++m_deviceAddressesFound;


        if (foundAddress.deviceFamily() == TMP1826Driver::DeviceFamilyCode) {
            DEBUG("Found on-board OneWire device: {}", foundAddress);
            m_onboardDevice.setAddress(foundAddress);
        } else if (m_externalDevicesFound < MaxExternalDevicesToFind &&
            foundAddress.deviceFamily() == DS18B20Driver::DeviceFamilyCode) {
            DEBUG("Found external OneWire device: {}", foundAddress);
            m_foundExternalDevices[m_externalDevicesFound].setAddress(foundAddress);
            ++m_externalDevicesFound;
        } else {
            WARN("Found unexpected OneWire device: {}", foundAddress);
        }

        if (result.isLastDevice || m_deviceAddressesFound >= m_foundDeviceAddresses.size()) {
            DEBUG("Done finding devices");
            if (m_onboardDevice.address()) {
                m_state = ReadOnboardDeviceScratchpadCommandStart;
                return;
            }
            if (m_deviceAddressesFound > 0) {
                m_state = RequestTemperatureConversionStart;
                return;
            }
            WARN("No temperature sensors found!");
            m_state = Invalid;
            return;
        }
        m_state = ScanForDevicesStart;
    }


    void OneWireSubsystem::readOnboardDeviceScratchpadCommandStartState() {
        if (!m_onboardDevice.startReadScratchpadCommand()) {
            wait(State::ReadOnboardDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadOnboardDeviceScratchpadCommandComplete, DS2485Driver::WriteBlockTimeUs);
    }

    void OneWireSubsystem::readOnboardDeviceScratchpadCommandCompleteState() {
        if (!m_onboardDevice.completeReadScratchpadCommand()) {
            wait(State::ReadOnboardDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }
        m_state = State::ReadOnboardDeviceScratchpadStart;
    }

    void OneWireSubsystem::readOnboardDeviceScratchpadStartState() {
        if (!m_onboardDevice.startReadScratchpad()) {
            wait(State::ReadOnboardDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadOnboardDeviceScratchpadComplete, DS2485Driver::ReadBlockTimeUs);
    }

    void OneWireSubsystem::readOnboardDeviceScratchpadCompleteState() {
        using enum State;
        if (!m_onboardDevice.completeReadScratchpad()) {
            wait(ReadOnboardDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }

        if (!m_onboardDevice.configurationIsValid()) {
            DEBUG("Setting onboard device configuration");
            m_state = WriteOnboardDeviceConfigStart;
            return;
        }
        if (m_externalDevicesFound > 0) {
            m_state = RequestTemperatureConversionStart;
            return;
        }
        wait(ReadOnboardDeviceScratchpadCommandStart, OnboardDeviceOnlyReadDelayUs);
    }

    void OneWireSubsystem::writeOnboardDeviceConfigStartState() {
        if (!m_onboardDevice.startWriteScratchpadCommand()) {
            wait(State::ReadOnboardDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::WriteOnboardDeviceConfigComplete, DS2485Driver::WriteBlockTimeUs * 10);
    }

    void OneWireSubsystem::writeOnboardDeviceConfigCompleteState() {
        if (!m_onboardDevice.completeWriteScratchpadCommand()) {
            wait(State::ReadOnboardDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadOnboardDeviceScratchpadCommandStart, DS2485Driver::ReadBlockTimeUs);
    }

    void OneWireSubsystem::requestTemperatureConversionStartState() {
        if (!m_foundExternalDevices[m_currentExternalDevice].startRequestTemperatureConversion()) {
            wait(State::RequestTemperatureConversionStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::RequestTemperatureConversionComplete, DS2485Driver::WriteBlockTimeUs);
    }

    void OneWireSubsystem::requestTemperatureConversionCompleteState() {
        if (!m_foundExternalDevices[m_currentExternalDevice].completeRequestTemperatureConversion()) {
            wait(State::RequestTemperatureConversionStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadDeviceScratchpadCommandStart, DS18B20Driver::ConversionTimeMs * 1000);
    }

    void OneWireSubsystem::readDeviceScratchpadCommandStartState() {
        if (!m_foundExternalDevices[m_currentExternalDevice].startReadScratchpadCommand()) {
            wait(State::ReadDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadDeviceScratchpadCommandComplete, DS2485Driver::WriteBlockTimeUs);
    }

    void OneWireSubsystem::readDeviceScratchpadCommandCompleteState() {
        if (!m_foundExternalDevices[m_currentExternalDevice].completeReadScratchpadCommand()) {
            wait(State::ReadDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }
        m_state = State::ReadDeviceScratchpadStart;
    }

    void OneWireSubsystem::readDeviceScratchpadStartState() {
        if (!m_foundExternalDevices[m_currentExternalDevice].startReadScratchpad()) {
            wait(State::ReadDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }

        wait(State::ReadDeviceScratchpadComplete, DS2485Driver::ReadBlockTimeUs);
    }

    void OneWireSubsystem::readDeviceScratchpadCompleteState() {
        using enum State;
        if (!m_foundExternalDevices[m_currentExternalDevice].completeReadScratchpad()) {
            wait(ReadDeviceScratchpadCommandStart, ErrorRetryDelayUs);
            return;
        }
        ++m_currentExternalDevice;
        if (m_currentExternalDevice >= m_externalDevicesFound) {
            m_currentExternalDevice = 0;
        }
        m_state = ReadOnboardDeviceScratchpadCommandStart;
    }
}
