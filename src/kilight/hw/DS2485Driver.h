/**
 * DS2485Driver.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <pico/error.h>

#include <mpf/util/macros.h>
#include <mpf/core/Logging.h>

#include "kilight/hw/I2CDevice.h"
#include "kilight/hw/onewire_address.h"

namespace kilight::hw {
    class DS2485Driver final : private I2CDevice {
        LOGGER(DS2485);

        static constexpr uint8_t I2CAddress = 0b1000000;

        enum class Command : uint8_t {
            Invalid = 0x00,
            WriteMemory = 0x96, //Write memory page
            ReadMemory = 0x44, // Read memory page
            SetPageProtection = 0xC3, // Set page protection of a memory page
            ReadStatus = 0xAA, // Read the protection for a memory page
            SetI2CAddress = 0x75, // Set the I2C address
            ReadOneWirePortConfig = 0x52, // Read all or one port configuration registers
            WriteOneWirePortConfig = 0x99, // Write to a 1-Wire port configuration register
            MasterReset = 0x62, // Reset the 1-Wire master block and return to defaults
            OneWireScript = 0x88, // Execute one or more 1-Wire primitive commands
            OneWireBlock = 0xAB, // Read and write 1-Wire data block (optional 1-Wire reset first and SPU at end)
            OneWireReadBlock = 0x50, // Read a block of 1-Wire data
            OneWireWriteBlock = 0x68, // Write 1-Wire block of data (optional 1-Wire reset first and SPU at end)
            OneWireSearch = 0x11, // Perform 1-Wire search algorithm
            FullCommandSequence = 0x57, // Performs a complete 1-Wire authenticator communication sequence
            ComputeCRC16 = 0xCC // Compute CRC16 over provided data
        };

        enum class OneWirePortConfigRegister : uint8_t {
            MasterConfiguration = 0x00,
            StandardSpeedTRSTL = 0x01,
            StandardSpeedTMSI = 0x02,
            StandardSpeedTMSP = 0x03,
            StandardSpeedTRSTH = 0x04,
            StandardSpeedTW0L = 0x05,
            StandardSpeedTW1L = 0x06,
            StandardSpeedTMSR = 0x07,
            StandardSpeedTREC = 0x08,
            OverdriveSpeedTRSTL = 0x09,
            OverdriveSpeedTMSI = 0x0A,
            OverdriveSpeedTMSP = 0x0B,
            OverdriveSpeedTRSTH = 0x0C,
            OverdriveSpeedTW0L = 0x0D,
            OverdriveSpeedTW1L = 0x0E,
            OverdriveSpeedTMSR = 0x0F,
            OverdriveSpeedTREC = 0x10,
            RPUPBUF = 0x11,
            PDSLEW = 0x12,
            Reserved = 0x13,
            All = 0xFF
        };

        template<Command commandToSend, typename CommandDataT>
        struct PACKED command_write_t {
            Command const command: 8 = commandToSend;
            uint8_t const writeLength: 8 = sizeof(CommandDataT);
            CommandDataT data;

            explicit command_write_t(CommandDataT const &data) : data(data) {
            }
        };

        struct PACKED command_result_t {
            uint8_t readLength = 0;
            uint8_t resultByte = 0;
        };

        template<typename ReadDataT>
        struct PACKED command_read_t {
            command_result_t header;
            ReadDataT readData;
        };

        template<OneWirePortConfigRegister registerToWrite, typename RegisterDataT>
        struct PACKED write_register_command_data_t {
            OneWirePortConfigRegister const reg: 8 = registerToWrite;
            RegisterDataT newValue;

            explicit write_register_command_data_t(RegisterDataT const &newValue) : newValue(newValue) {
            }
        };

    public:
        static constexpr uint64_t OperationDelayTimeUs = 400;

        static constexpr uint64_t SequenceTimeUs = 10;

        static constexpr uint64_t OneWireTime = 68 + 6;

        static constexpr uint64_t ResetTimeUs = 560 * 2;

        static constexpr uint64_t SearchDelayTimeUs = OperationDelayTimeUs + SequenceTimeUs * 65 + OneWireTime + ResetTimeUs + 100000;

        static constexpr uint64_t WriteBlockTimeUs = OperationDelayTimeUs + SequenceTimeUs * (3 + 8) + OneWireTime + ResetTimeUs + 10000;

        static constexpr uint64_t ReadBlockTimeUs = OperationDelayTimeUs + SequenceTimeUs * (10 + 8) + OneWireTime + ResetTimeUs + 10000;

        enum class OneWireRWPU : uint8_t {
            External = 0b00,
            FiveHundred = 0b01,
            OneThousand = 0b10,
            ThreeThirtyThree = 0b11
        };

        enum class OneWireVIAPO : uint8_t {
            Low = 0b00,
            Medium = 0b01,
            High = 0b10,
            Off = 0b11
        };

        enum class OneWireVTH : uint8_t {
            Low = 0b00,
            Medium = 0b01,
            High = 0b10,
            Off = 0b11
        };

        struct PACKED master_configuration_t {
            unsigned: 12;
            bool activePullUp: 1;
            bool strongPullUp: 1;
            bool powerDown: 1;
            bool oneWireOverdriveSpeed: 1;
        };

        struct PACKED rpupbuf_configuration_t {
            OneWireRWPU rwpu: 2 = OneWireRWPU::External;
            OneWireVIAPO viapo: 2 = OneWireVIAPO::Off;
            OneWireVTH vth: 2 = OneWireVTH::Off;
            unsigned: 9;
            bool custom: 1 = true;
        };

        struct PACKED onewire_search_params_t {
            bool reset: 1 = false;
            bool ignorePresencePulse: 1 = false;
            bool resetSearch: 1 = false;
            unsigned: 5;
            unsigned searchCommandCode: 8 = 0xF0;
        };

        struct PACKED onewire_search_result_t {
            onewire_address_t address = 0;
            bool isLastDevice: 1 = false;
            unsigned: 7;
        };

        struct PACKED onewire_write_block_params_t {
            bool reset: 1 = true;
            bool ignorePresencePulse: 1 = false;
            bool strongPullUp: 1 = false;
            unsigned: 5;
        };

        template<typename OneWireDataT>
        struct PACKED onewire_write_block_request_t {
            onewire_write_block_params_t writeBlockParams {};
            OneWireDataT data;

            explicit onewire_write_block_request_t(OneWireDataT const & data) : data(data) {}
        };

        using onewire_read_length_t = uint8_t;

        DS2485Driver();

        [[nodiscard]]
        bool startReadMasterConfiguration() const;

        [[nodiscard]]
        bool completeReadMasterConfiguration(master_configuration_t *configurationOut) const;

        [[nodiscard]]
        bool startWriteMasterConfiguration(master_configuration_t const &newConfiguration) const;

        [[nodiscard]]
        bool completeWriteMasterConfiguration() const;

        [[nodiscard]]
        bool startReadRPUPBUFConfiguration() const;

        [[nodiscard]]
        bool completeReadRPUPBUFConfiguration(rpupbuf_configuration_t *configurationOut) const;

        [[nodiscard]]
        bool startWriteRPUPBUFConfiguration(rpupbuf_configuration_t const &newConfiguration) const;

        [[nodiscard]]
        bool completeWriteRPUPBUFConfiguration() const;

        [[nodiscard]]
        bool startOneWireSearch(onewire_search_params_t const &searchParams) const;

        [[nodiscard]]
        bool completeOneWireSearch(onewire_search_result_t *searchResult) const;

        template<typename OneWireDataT>
        [[nodiscard]]
        bool startOneWireWriteBlockData(OneWireDataT const &writeData) {
            return startOneWireWriteBlock(onewire_write_block_request_t<OneWireDataT> { writeData });
        }

        template<typename OneWireDataT>
        [[nodiscard]]
        bool startOneWireWriteBlock(onewire_write_block_request_t<OneWireDataT> const &writeBlock) {
            onewire_write_block_command_t<OneWireDataT> const command{
                writeBlock
            };
            return sendCommand(command);
        }

        [[nodiscard]]
        bool completeOneWireWriteBlock() const;

        [[nodiscard]]
        bool startOneWireReadBlock(onewire_read_length_t readLength) const;

        template<typename OneWireDataT>
        [[nodiscard]]
        bool completeOneWireReadBlock(OneWireDataT *const dataOut) const {
            onewire_read_block_command_result_t<OneWireDataT> result{};
            if (!readResult(&result)) {
                WARN("NACK on OneWire read block read-back");
                return false;
            }

            switch (result.header.resultByte) {
                case 0xAA:
                    *dataOut = result.readData;
                    return true;

                case 0x77:
                    WARN("Invalid parameter error code returned from read block read-back");
                    return false;

                case 0x22:
                    WARN("Communication failure error code returned from read block read-back");
                    return false;

                default:
                    WARN("Unknown error code {} returned during OneWire read block read-back", result.header.resultByte);
                    return false;
            }
        }

    private:
        using read_onewire_port_config_register_command_t =
        command_write_t<Command::ReadOneWirePortConfig, OneWirePortConfigRegister>;

        using read_onewire_port_config_register_master_configuration_result_t = command_read_t<master_configuration_t>;

        using write_master_configuration_register_data_t =
        write_register_command_data_t<OneWirePortConfigRegister::MasterConfiguration, master_configuration_t>;

        using write_onewire_port_config_register_master_configuration_command_t =
        command_write_t<Command::WriteOneWirePortConfig, write_master_configuration_register_data_t>;


        using read_onewire_port_config_register_rpupbuf_configuration_result_t =
        command_read_t<rpupbuf_configuration_t>;

        using write_rpupbuf_configuration_register_data_t =
        write_register_command_data_t<OneWirePortConfigRegister::RPUPBUF, rpupbuf_configuration_t>;

        using write_onewire_port_config_register_rpupbuf_configuration_command_t =
        command_write_t<Command::WriteOneWirePortConfig, write_rpupbuf_configuration_register_data_t>;


        using onewire_search_command_t = command_write_t<Command::OneWireSearch, onewire_search_params_t>;

        using onewire_search_command_result_t = command_read_t<onewire_search_result_t>;


        template<typename OneWireDataT>
        using onewire_write_block_command_t =
            command_write_t<Command::OneWireWriteBlock, onewire_write_block_request_t<OneWireDataT> >;


        using onewire_read_block_command_t = command_write_t<Command::OneWireReadBlock, onewire_read_length_t>;

        template<typename OneWireDataT>
        using onewire_read_block_command_result_t = command_read_t<OneWireDataT>;


        template<Command commandToSend, typename CommandDataT>
        [[nodiscard]]
        bool sendCommand(command_write_t<commandToSend, CommandDataT> const &command) const {
            int const result = write(command);
            if (result > 0) {
                return true;
            }
            if (result == PICO_ERROR_GENERIC) {
                WARN("Got I2C NACK error trying to send command");
                return false;
            }
            if (result == PICO_ERROR_TIMEOUT) {
                WARN("Got I2C timeout error trying to send command");
                return false;
            }
            WARN("Got error {} trying to send command", result);
            return false;
        }

        [[nodiscard]]
        bool readResult(command_result_t *const result) const {
            int const resultCode = read(result);
            if (resultCode > 0) {
                return true;
            }
            if (resultCode == PICO_ERROR_GENERIC) {
                WARN("Got I2C NACK error trying to read result");
                return false;
            }
            if (resultCode == PICO_ERROR_TIMEOUT) {
                WARN("Got I2C timeout error trying to read result");
                return false;
            }
            WARN("Got error {} trying to read result", resultCode);
            return false;
        }

        template<typename ReadDataT>
        [[nodiscard]]
        bool readResult(command_read_t<ReadDataT> *const result) const {
            int const resultCode = read(result);
            if (resultCode > 0) {
                return true;
            }
            if (resultCode == PICO_ERROR_GENERIC) {
                WARN("Got I2C NACK error trying to read result");
                return false;
            }
            if (resultCode == PICO_ERROR_TIMEOUT) {
                WARN("Got I2C timeout error trying to read result");
                return false;
            }
            WARN("Got error {} trying to read result", resultCode);
            return false;
        }
    };
}
