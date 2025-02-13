/**
 * I2CDevice.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <span>

#include <cassert>

namespace kilight::hw {

    class I2CDevice {
    public:
        static constexpr uint32_t ReadTimeoutUs = 10 * 1000;
        static constexpr uint32_t WriteTimeoutUs = 10 * 1000;

        explicit I2CDevice(uint8_t address);

        virtual ~I2CDevice() = default;

        template<typename ReadT>
        [[nodiscard]]
        int read(ReadT * const dest, bool const noStop = false) const {
            assert(dest != nullptr);
            std::array<uint8_t, sizeof(ReadT)> buffer {};
            int const result = read(std::span<uint8_t>{buffer}, noStop);
            std::memcpy(dest, buffer.data(), sizeof(ReadT));
            return result;
        }

        [[nodiscard]]
        int read(std::span<uint8_t> const & buffer, bool noStop = false) const;

        template<typename WriteT>
        [[nodiscard]]
        int write(WriteT const & src, bool const noStop = false) const {
            std::array<uint8_t, sizeof(WriteT)> buffer {};
            std::memcpy(buffer.data(), &src, sizeof(WriteT));
            return write(std::span<uint8_t const>{buffer}, noStop);
        }

        [[nodiscard]]
        int write(std::span<uint8_t const> const & buffer, bool noStop = false) const;

        template<typename ReadT>
        [[nodiscard]]
        int readRegister(uint8_t const registerAddress, ReadT * const dest, bool const noStop = false) const {
            assert(dest != nullptr);
            std::array<uint8_t, sizeof(ReadT)> buffer {};
            int const result = readRegister(registerAddress, std::span<uint8_t>{buffer}, noStop);
            std::memcpy(dest, buffer.data(), sizeof(ReadT));
            return result;
        }

        [[nodiscard]]
        int readRegister(uint8_t registerAddress, std::span<uint8_t> const & buffer, bool noStop = false) const;

    private:
        uint8_t const m_address;
    };

}
