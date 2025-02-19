/**
 * TMP1826Driver.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>
// ReSharper disable once CppUnusedIncludeDirective
#include <compare>

#include <mpf/util/macros.h>
#include <mpf/core/Logging.h>

#include "kilight/hw/OneWireDevice.h"
#include "kilight/hw/TemperatureSensor.h"

namespace kilight::hw {
    class TMP1826Driver final : public OneWireDevice, public TemperatureSensor {
        LOGGER(TMP1826);
    public:
        static constexpr uint32_t ConversionTimeMs = 7 * 8;

        static constexpr uint8_t DeviceFamilyCode = 0x26;

        explicit TMP1826Driver(DS2485Driver* busDriver);

        ~TMP1826Driver() override = default;

        [[nodiscard]]
        int16_t currentTemperature() const override;

        [[nodiscard]]
        bool configurationNeedsSetting() const;

        [[nodiscard]]
        bool configurationIsValid() const;

        [[nodiscard]]
        bool startReadScratchpadCommand() const;

        [[nodiscard]]
        bool completeReadScratchpadCommand() const;

        [[nodiscard]]
        bool startReadScratchpad() const;

        [[nodiscard]]
        bool completeReadScratchpad();

        [[nodiscard]]
        bool startWriteScratchpadCommand() const;

        [[nodiscard]]
        bool completeWriteScratchpadCommand() const;

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
            OverdriveSkipRom = 0x3C,
            FlexAddr = 0x0F,
            AlarmSearch = 0xEC
        };

        enum class Command : uint8_t {
            ConvertT = 0x44,
            WriteScratchPad = 0x4E,
            ReadScratchPad = 0xBE,
            CopyScratchPad = 0x48
        };

        enum class ConversionMode : uint8_t {
            OneShot = 0b000,
            Every8Seconds = 0b001,
            Every4Seconds = 0b010,
            Every2Seconds = 0b011,
            EverySecond = 0b100,
            EveryHalfSecond = 0b101,
            EveryQuarterSecond = 0b110,
            EveryEighthSecond = 0b111
        };

        enum class AlertHysteresis : uint8_t {
            FiveDegrees = 0b00,
            TenDegrees = 0b01,
            FifteenDegrees = 0b10,
            TwentyDegrees = 0b11
        };

        enum class ArbitrationMode : uint8_t {
            Disabled = 0b00,
            Enabled = 0b10,
            FastEnabled = 0b11
        };

        enum class FlexAddressMode : uint8_t {
            Host = 0b00,
            PinDecode = 0b01,
            Resistor = 0b10,
            PinAndResistor = 0b11
        };

        struct PACKED configuration_register_t {
            ConversionMode conversionMode: 3 = ConversionMode::OneShot;
            bool averageEightConversions: 1 = false;
            bool alertPinMode: 1 = false;
            bool useLongConversionTime: 1 = false;
            unsigned: 1;
            bool useSixteenBitTemperatureFormat: 1 = false;

            bool registerProtectionLockEnabled: 1 = false;
            AlertHysteresis alertHysteresis: 2 = AlertHysteresis::FiveDegrees;
            ArbitrationMode arbitrationMode: 2 = ArbitrationMode::Disabled;
            FlexAddressMode flexAddressMode: 2 = FlexAddressMode::Host;
            bool overdriveEnabled: 1 = true;

            auto operator<=>(configuration_register_t const & other) const = default;
        };

        struct PACKED status_register_t {
            bool lockStatus: 1 = false;
            bool arbitrationDone: 1 = false;
            bool powerMode: 1 = false;
            bool dataValid: 1 = false;
            unsigned: 2;
            bool alertLow: 1 = false;
            bool alertHigh: 1 = false;
        };

        struct PACKED scratchpad_t {
            int16_t temperature: 16 = 0;

            status_register_t status {};

            unsigned: 8;

            configuration_register_t configurationRegister {};

            uint8_t shortAddress: 8 = 0;

            unsigned: 8;

            uint8_t firstCRC: 8 = 0;

            int16_t temperatureAlertLow: 16 = 0;

            int16_t temperatureAlertHigh: 16 = 0;

            int16_t temperatureOffset: 16 = 0;

            unsigned: 16;

            uint8_t lastCRC: 8 = 0;
        };


        struct PACKED scratchpad_write_t {
            configuration_register_t configurationRegister {};

            uint8_t shortAddress: 8 = 0;

            int16_t temperatureAlertLow: 16 = 0;

            int16_t temperatureAlertHigh: 16 = 0;

            int16_t temperatureOffset: 16 = 0;
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

        using write_scratchpad_command_t = match_rom_command_with_data_t<Command::WriteScratchPad, scratchpad_write_t>;

        using copy_scratchpad_command_t = match_rom_command_t<Command::CopyScratchPad>;

        using convert_temperature_command_t = match_rom_command_t<Command::ConvertT>;

        static constexpr configuration_register_t ConfigurationToSet {
            .conversionMode = ConversionMode::EveryQuarterSecond,
            .averageEightConversions = true,
            .alertPinMode = false,
            .useLongConversionTime = true,
            .useSixteenBitTemperatureFormat = true,
            .registerProtectionLockEnabled = false,
            .alertHysteresis = AlertHysteresis::FiveDegrees,
            .arbitrationMode = ArbitrationMode::Disabled,
            .flexAddressMode = FlexAddressMode::Host,
            .overdriveEnabled = false
        };

        scratchpad_t m_scratchpad {};
    };
}
