/**
 * Pin
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>
#include <functional>
#include <array>
#include <hardware/platform_defs.h>

namespace kilight::hw {

    enum class PinFunction {
        SPI,
        UART,
        I2C,
        PWM,
        SIO,
        PIO0,
        PIO1,
        GPCK,
        USB,
        None
    };

    enum class GPIOInterruptTrigger : uint32_t {
        None = 0U,
        LevelLow = 1U,
        LevelHigh = 1U << 1U,
        EdgeFalling = 1U << 2U,
        EdgeRising = 1U << 3U
    };

    enum class GPIOPullDirection {
        None,
        PullUp,
        PullDown
    };

    constexpr GPIOInterruptTrigger operator|(GPIOInterruptTrigger const first, GPIOInterruptTrigger const second) {
        return static_cast<GPIOInterruptTrigger>(static_cast<uint32_t>(first) | static_cast<uint32_t>(second));
    }

    constexpr GPIOInterruptTrigger operator&(GPIOInterruptTrigger const first, GPIOInterruptTrigger const second) {
        return static_cast<GPIOInterruptTrigger>(static_cast<uint32_t>(first) | static_cast<uint32_t>(second));
    }

    using GPIOInterruptCallback = std::function<void(uint8_t gpioNumber, GPIOInterruptTrigger trigger)>;

    class GPIOWrapper {
    public:
        GPIOWrapper() = delete;
        ~GPIOWrapper() = delete;

    protected:
        static void initPin(uint8_t gpioNumber, PinFunction pinFunction);

        static void setDirection(uint8_t gpioNumber, bool output);

        static void setPull(uint8_t gpioNumber, GPIOPullDirection direction);

        static void write(uint8_t gpioNumber, bool val);

        [[nodiscard]]
        static bool read(uint8_t gpioNumber);

        static void toggle(uint8_t gpioNumber);

        static void enablePWM(uint8_t gpioNumber, uint32_t frequencyHz, uint16_t top);

        static void writePWM(uint8_t gpioNumber, uint16_t value);

        static void setInterrupt(uint8_t gpioNumber, GPIOInterruptTrigger trigger, GPIOInterruptCallback const & callback);

        static void enableADC(uint8_t gpioNumber);

        static uint16_t readADC(uint8_t adcChannel);

    private:
        inline static std::array<GPIOInterruptCallback, 30> interrupts {};

        static void interruptWrapper(unsigned int gpio, uint32_t trigger);
    };

    template<uint8_t gpioNumber, PinFunction pinFunction>
    class Pin : protected GPIOWrapper {
    public:
        static constexpr uint8_t Number = gpioNumber;

        static constexpr PinFunction Function = pinFunction;

        Pin() = delete;

        static void initPin() {
            if constexpr (Function != PinFunction::None) {
                GPIOWrapper::initPin(Number, Function);
            }
        }
    };

    template<uint8_t gpioNumber>
    class GPIOPin : public Pin<gpioNumber, PinFunction::SIO> {
    public:
        static void setDirection(bool const output = true) {
            GPIOWrapper::setDirection(gpioNumber, output);
        }

        static void setOutput() {
            setDirection(true);
        }

        static void setInput() {
            setDirection(false);
        }

        static void setPull(GPIOPullDirection const direction) {
            GPIOWrapper::setPull(gpioNumber, direction);
        }

        static void setPullUp() {
            setPull(GPIOPullDirection::PullUp);
        }

        static void setPullDown() {
            setPull(GPIOPullDirection::PullDown);
        }

        static void setPullNone() {
            setPull(GPIOPullDirection::None);
        }

        [[nodiscard]]
        static bool read() {
            return GPIOWrapper::read(gpioNumber);
        }

        static void write(bool const val = true) {
            return GPIOWrapper::write(gpioNumber, val);
        }

        static void toggle() {
            GPIOWrapper::toggle(gpioNumber);
        }

        static void setLow() {
            write(false);
        }

        static void setHigh() {
            write(true);
        }

        static void setInterrupt(GPIOInterruptTrigger const trigger, GPIOInterruptCallback const & callback) {
            GPIOWrapper::setInterrupt(gpioNumber, trigger, callback);
        }

        GPIOPin() = delete;
    };

    template<uint8_t gpioNumber>
    class UARTPin : public Pin<gpioNumber, PinFunction::UART> {
    public:
        UARTPin() = delete;
    };

    template<uint8_t gpioNumber>
    class SPIPin : public Pin<gpioNumber, PinFunction::SPI> {
    public:
        SPIPin() = delete;
    };

    template<uint8_t gpioNumber>
    class I2CPin : public Pin<gpioNumber, PinFunction::I2C> {
    public:
        I2CPin() = delete;
    };

    template<uint8_t gpioNumber>
    class ADCPin : public Pin<gpioNumber, PinFunction::None> {
    public:
        static constexpr uint8_t ADCChannel = gpioNumber - ADC_BASE_PIN;

        ADCPin() = delete;

        static void initPin() {
            GPIOWrapper::enableADC(gpioNumber);
        }

        static uint16_t readADC() {
            return GPIOWrapper::readADC(ADCChannel);
        }

    };

    template<uint8_t gpioNumber>
    class PWMPin : public Pin<gpioNumber, PinFunction::PWM> {
        static constexpr uint16_t PWMCount = 1000;
        static constexpr uint16_t PWMTop = PWMCount - 1;

    public:
        static void enablePWM(uint32_t const frequencyHz) {
            GPIOWrapper::enablePWM(gpioNumber, frequencyHz, PWMTop);
        }

        static void writePercent(uint8_t const percent) {
            writePerThou(static_cast<uint16_t>(percent) * 10);
        }

        static void writePerThou(uint16_t perThou) {
            if (perThou > 1000) {
                perThou = 1000;
            }
            GPIOWrapper::writePWM(gpioNumber, perThou);
        }

        PWMPin() = delete;
    };

    template<uint8_t gpioNumber>
    class PWM8Pin : public Pin<gpioNumber, PinFunction::PWM> {
        static constexpr uint16_t PWMCount = 256;
        static constexpr uint16_t PWMTop = PWMCount - 1;

    public:
        static void enablePWM(uint32_t const frequencyHz) {
            GPIOWrapper::enablePWM(gpioNumber, frequencyHz, PWMTop);
        }

        static void writePWM(uint8_t const value) {
            GPIOWrapper::writePWM(gpioNumber, value);
        }

        PWM8Pin() = delete;
    };

    template<uint8_t gpioNumber>
    class UnusedPin : public Pin<gpioNumber, PinFunction::None> {
    public:
        UnusedPin() = delete;
    };
}
