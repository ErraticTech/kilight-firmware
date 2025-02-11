/**
 * SystemPins
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>

#include "kilight/hw/Pin.h"

namespace kilight {
    class KiLight;
}

namespace kilight::hw {
    template<uint8_t redPin,
             uint8_t greenPin,
             uint8_t bluePin,
             uint8_t cwPin,
             uint8_t wwPin,
             uint8_t adcPin,
             uint32_t pwmFrequency = 32000>
    class ChannelPins {
    public:
        static constexpr uint32_t PWMFrequency = pwmFrequency;

        using Red = PWM8Pin<redPin>;

        using Green = PWM8Pin<greenPin>;

        using Blue = PWM8Pin<bluePin>;

        using ColdWhite = PWM8Pin<cwPin>;

        using WarmWhite = PWM8Pin<wwPin>;

        using CurrentSense = ADCPin<adcPin>;

        static void enablePWM() {
            Red::enablePWM(PWMFrequency);
            Green::enablePWM(PWMFrequency);
            Blue::enablePWM(PWMFrequency);
            ColdWhite::enablePWM(PWMFrequency);
            WarmWhite::enablePWM(PWMFrequency);
        }
    };

    class SystemPins final {
        friend class KiLight;

    public:

        // region LED Channels
        using OutputA = ChannelPins<0, 2, 4, 6, 8, 26>;
        // endregion

        // region Fan
        using FanTacho = GPIOPin<11>;
        using FanPWM = PWMPin<12>;
        // endregion

        // region Button
        using ClearButton = GPIOPin<15>;
        // endregion

        // region Debug UART
        using DebugUARTTxD = UARTPin<16>;
        using DebugUARTRxD = UARTPin<17>;
        // endregion

        // region LED
        using ActivityLED = GPIOPin<18>;
        using StatusLED = GPIOPin<19>;
        // endregion

        // region I2C
        using SDA = I2CPin<20>;
        using SCL = I2CPin<21>;
        // endregion

        // region Unused Pins
        using UnusedPinA = UnusedPin<1>;
        using UnusedPinB = UnusedPin<3>;
        using UnusedPinC = UnusedPin<5>;
        using UnusedPinD = UnusedPin<7>;
        using UnusedPinE = UnusedPin<9>;
        using UnusedPinF = UnusedPin<10>;
        using UnusedPinG = UnusedPin<13>;
        using UnusedPinH = UnusedPin<14>;
        using UnusedPinI = UnusedPin<22>;
        using UnusedPinJ = UnusedPin<27>;
        using UnusedPinK = UnusedPin<28>;
        // endregion

        SystemPins() = delete;

        ~SystemPins() = delete;

    private:
        static void initPins();
    };
}
