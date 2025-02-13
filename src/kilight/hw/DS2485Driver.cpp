/**
 * DS2485Driver.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/DS2485Driver.h"

namespace kilight::hw {
    DS2485Driver::DS2485Driver() : I2CDevice(I2CAddress) {
    }

    bool DS2485Driver::startReadMasterConfiguration() const {
        read_onewire_port_config_register_command_t const command {
            OneWirePortConfigRegister::MasterConfiguration
        };
        return sendCommand(command);
    }

    bool DS2485Driver::completeReadMasterConfiguration(master_configuration_t * const configurationOut) const {
        read_onewire_port_config_register_master_configuration_result_t result;
        if (!readResult(&result)) {
            WARN("NACK on Read Master Configuration");
            return false;
        }
        if (result.header.resultByte != 0xAA) {
            WARN("Bad result byte on Read Master Configuration: {:#04X}", result.header.resultByte);
            return false;
        }
        *configurationOut = result.readData;
        TRACE("Read master configuration");
        return true;
    }

    bool DS2485Driver::startWriteMasterConfiguration(master_configuration_t const &newConfiguration) const {
        write_onewire_port_config_register_master_configuration_command_t const command {
            write_master_configuration_register_data_t {
                newConfiguration
            }
        };
        return sendCommand(command);
    }

    bool DS2485Driver::completeWriteMasterConfiguration() const {
        command_result_t result;
        if (!readResult(&result)) {
            WARN("NACK on Write Master Configuration Status read-back");
            return false;
        }

        switch (result.resultByte) {
            case 0xAA:
                TRACE("Wrote master configuration");
                return true;

            case 0x77:
                WARN("Invalid parameter error returned on Write Master Configuration Status read-back");
                return false;

            default:
                WARN("Unknown error code {:#04X} returned on Write Master Configuration Status read-back",
                     result.resultByte);
                return false;
        }
    }

    bool DS2485Driver::startReadRPUPBUFConfiguration() const {
        read_onewire_port_config_register_command_t const command {
            OneWirePortConfigRegister::RPUPBUF
        };
        return sendCommand(command);
    }

    bool DS2485Driver::completeReadRPUPBUFConfiguration(rpupbuf_configuration_t * const configurationOut) const {
        read_onewire_port_config_register_rpupbuf_configuration_result_t result;
        if (!readResult(&result)) {
            WARN("NACK on Read RPUPBUF Configuration");
            return false;
        }
        if (result.header.resultByte != 0xAA) {
            WARN("Bad result byte on Read RPUPBUF Configuration: {:#04X}", result.header.resultByte);
            return false;
        }
        *configurationOut = result.readData;
        TRACE("Read RPUPBUF configuration");
        return true;
    }

    bool DS2485Driver::startWriteRPUPBUFConfiguration(rpupbuf_configuration_t const &newConfiguration) const {
        write_onewire_port_config_register_rpupbuf_configuration_command_t const command {
            write_rpupbuf_configuration_register_data_t {
                newConfiguration
            }
        };
        return sendCommand(command);
    }

    bool DS2485Driver::completeWriteRPUPBUFConfiguration() const {
        command_result_t result;
        if (!readResult(&result)) {
            WARN("NACK on Write RPUPBUF Configuration Status read-back");
            return false;
        }

        switch (result.resultByte) {
            case 0xAA:
                TRACE("Wrote RPUPBUF configuration");
                return true;

            case 0x77:
                WARN("Invalid parameter error returned on Write RPUPBUF Configuration Status read-back");
                return false;

            default:
                WARN("Unknown error code {:#04X} returned on Write RPUPBUF Configuration Status read-back",
                     result.resultByte);
                return false;
        }
    }

    bool DS2485Driver::startOneWireSearch(onewire_search_params_t const &searchParams) const {
        onewire_search_command_t const command {
            searchParams
        };
        TRACE("Starting OneWire bus search...");
        return sendCommand(command);
    }

    bool DS2485Driver::completeOneWireSearch(onewire_search_result_t * const searchResult) const {
        onewire_search_command_result_t result;
        if (!readResult(&result)) {
            WARN("NACK on OneWire search command read-back");
            return false;
        }

        switch (result.header.resultByte) {
            case 0xAA:
                *searchResult = result.readData;
                return true;

            case 0x33:
                WARN("OneWire presence pulse not detected during search");
                return false;

            case 0x00:
                WARN("OneWire device not detected during search");
                return false;

            case 0x77:
                WARN("OneWire search returned invalid parameter error");
                return false;

            default:
                WARN("Unknown error code {:#04X} returned during OneWire search read-back", result.header.resultByte);
                return false;
        }
    }

    bool DS2485Driver::completeOneWireWriteBlock() const {
        command_result_t result;
        if (!readResult(&result)) {
            WARN("NACK on OneWire write block read-back");
            return false;
        }

        switch (result.resultByte) {
            case 0xAA:
                TRACE("OneWire write block complete");
                return true;

            case 0x22:
                WARN("Communication failure error returned on OneWire write block read-back");
                return false;

            case 0x33:
                WARN("OneWire presence pulse not detected error returned on OneWire write block read-back");
                return false;

            case 0x00:
                WARN("Non-matching OneWire writes error returned on OneWire write block read-back");
                return false;

            case 0x77:
                WARN("Invalid parameter error returned on OneWire write block read-back");
                return false;

            default:
                WARN("Unknown error code {:#04X} returned on OneWire write block read-back", result.resultByte);
                return false;
        }
    }

    bool DS2485Driver::startOneWireReadBlock(onewire_read_length_t const readLength) const {
        onewire_read_block_command_t const command {
            readLength
        };
        return sendCommand(command);
    }
}
