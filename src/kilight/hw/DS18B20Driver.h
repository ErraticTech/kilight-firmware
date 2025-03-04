/**
 * DS18B20Driver.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>

#include <mpf/util/macros.h>
#include <mpf/core/Logging.h>

#include "kilight/hw/OneWireDevice.h"
#include "kilight/hw/TemperatureSensor.h"

namespace kilight::hw {
    class DS18B20Driver final : public OneWireDevice, public TemperatureSensor {
        LOGGER(DS18B20);
    public:
        static constexpr uint32_t ConversionTimeMs = 750;

        static constexpr uint8_t DeviceFamilyCode = 0x28;

        enum class Resolution : uint8_t {
            NineBits = 0b00,
            TenBits = 0b01,
            ElevenBits = 0b10,
            TwelveBits = 0b11
        };

        explicit DS18B20Driver(DS2485Driver* driver);

        ~DS18B20Driver() override = default;

        [[nodiscard]]
        int16_t currentTemperature() const override;

        [[nodiscard]]
        bool startRequestTemperatureConversion() const;

        [[nodiscard]]
        bool completeRequestTemperatureConversion() const;

        [[nodiscard]]
        bool startReadScratchpadCommand() const;

        [[nodiscard]]
        bool completeReadScratchpadCommand() const;

        [[nodiscard]]
        bool startReadScratchpad() const;

        [[nodiscard]]
        bool completeReadScratchpad();

        [[nodiscard]]
        bool startCopyScratchpadCommand() const;

        [[nodiscard]]
        bool completeCopyScratchpadCommand() const;

    private:
        enum class ROMCommand : uint8_t {
            SearchRom = 0xF0,
            ReadRom = 0x33,
            MatchRom = 0x55,
            SkipRom = 0xCC,
            AlarmSearch = 0xEC
        };

        enum class Command : uint8_t {
            ConvertT = 0x44,
            WriteScratchPad = 0x4E,
            ReadScratchPad = 0xBE,
            CopyScratchPad = 0x48,
            RecallEEPROM = 0xB8,
            ReadPowerSupply = 0xB4
        };

        struct PACKED configuration_register_t {
            unsigned: 5;
            Resolution resolution: 2 = Resolution::TwelveBits;
            unsigned: 1;
        };

        struct PACKED scratchpad_t {
            int16_t temperature: 12 = 0;
            unsigned: 4;
            int8_t tHRegister = 0;
            int8_t tLRegister = 0;
            configuration_register_t config {};
            unsigned: 24;
            uint8_t crc = 0;
        };

        template<Command commandToRun>
        struct PACKED skip_rom_command_t {
            ROMCommand const skipRomCommand: 8 = ROMCommand::SkipRom;
            Command const command: 8 = commandToRun;
        };

        template<Command commandToRun>
        struct PACKED match_rom_command_t {
            ROMCommand const matchRomCommand: 8 = ROMCommand::MatchRom;
            onewire_address_t address;
            Command const command: 8 = commandToRun;

            explicit match_rom_command_t(onewire_address_t const address)
                : address(address) {}
        };

        template<Command commandToRun, typename CommandDataT>
        struct PACKED match_rom_command_with_data_t {
            ROMCommand const matchRomCommand: 8 = ROMCommand::MatchRom;
            onewire_address_t address;
            Command const command: 8 = commandToRun;
            CommandDataT commandData;

            match_rom_command_with_data_t(onewire_address_t const address,
                                          CommandDataT const &commandData)
                : address(address), commandData(commandData) {}
        };

        using read_scratchpad_command_t = match_rom_command_t<Command::ReadScratchPad>;

        using copy_scratchpad_command_t = match_rom_command_t<Command::CopyScratchPad>;

        using convert_temperature_command_t = match_rom_command_t<Command::ConvertT>;

        using convert_temperature_all_command_t = skip_rom_command_t<Command::ConvertT>;

        scratchpad_t m_scratchpad {};
    };
}
