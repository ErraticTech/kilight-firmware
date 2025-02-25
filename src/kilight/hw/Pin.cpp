/**
 * Pin
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/Pin.h"

#include <array>

#include <hardware/pwm.h>
#include <hardware/adc.h>

#include "kilight/hw/SysClock.h"

namespace kilight::hw {

    void GPIOWrapper::initPin(uint8_t const gpio, PinFunction const pinFunction) {
        using enum PinFunction;
        if (pinFunction == SIO) {
            gpio_init(gpio);
            return;
        }
        gpio_function_t mappedGPIOFunction = GPIO_FUNC_NULL;
        switch(pinFunction) {

            case SPI:
                mappedGPIOFunction = GPIO_FUNC_SPI;
                break;

            case UART:
                mappedGPIOFunction = GPIO_FUNC_UART;
                break;

            case I2C:
                mappedGPIOFunction = GPIO_FUNC_I2C;
                break;

            case PWM:
                mappedGPIOFunction = GPIO_FUNC_PWM;
                break;

            case PIO0:
                mappedGPIOFunction = GPIO_FUNC_PIO0;
                break;

            case PIO1:
                mappedGPIOFunction = GPIO_FUNC_PIO1;
                break;

            case GPCK:
                mappedGPIOFunction = GPIO_FUNC_GPCK;
                break;

            case USB:
                mappedGPIOFunction = GPIO_FUNC_USB;
                break;

            default: break;
        }

        gpio_set_function(gpio, mappedGPIOFunction);

    }

    void GPIOWrapper::setDirection(uint8_t const gpioNumber, bool const output) {
        gpio_set_dir(gpioNumber, output);
    }

    void GPIOWrapper::setPull(uint8_t const gpioNumber, GPIOPullDirection const direction) {
        switch (direction) {
        using enum GPIOPullDirection;
        case None:
            gpio_disable_pulls(gpioNumber);
            break;

        case PullUp:
            gpio_pull_up(gpioNumber);
            break;

        case PullDown:
            gpio_pull_down(gpioNumber);
            break;
        }
    }


    void GPIOWrapper::write(uint8_t const gpioNumber, bool const val) {
        gpio_put(gpioNumber, val);
    }

    bool GPIOWrapper::read(uint8_t const gpioNumber) {
        return gpio_get(gpioNumber);
    }

    void GPIOWrapper::toggle(uint8_t const gpioNumber) {
        gpioc_bit_out_xor(gpioNumber);
    }

    void GPIOWrapper::enablePWM(uint8_t const gpioNumber,
                                uint32_t const frequencyHz,
                                uint16_t const top) {
        float const maxFrequency = static_cast<float>(SysClock::Frequency) / static_cast<float>(top + 1);
        float const divider = maxFrequency / static_cast<float>(frequencyHz);
        auto const slice = pwm_gpio_to_slice_num(gpioNumber);
        auto const channel = pwm_gpio_to_channel(gpioNumber);
        pwm_set_wrap(slice, top);
        pwm_set_chan_level(slice, channel, 0);
        pwm_set_clkdiv(slice, divider);
        pwm_set_counter(slice, 0);
        pwm_set_enabled(slice, true);
    }

    void GPIOWrapper::writePWM(uint8_t const gpioNumber, uint16_t const value) {
        pwm_set_gpio_level(gpioNumber, value);
    }


    void GPIOWrapper::interruptWrapper(unsigned int const gpio, uint32_t const trigger) {
        if (interrupts[gpio]) {
            interrupts[gpio](static_cast<uint8_t>(gpio), static_cast<GPIOInterruptTrigger>(trigger));
        }
    }

    void GPIOWrapper::setInterrupt(uint8_t const gpioNumber,
                                   GPIOInterruptTrigger const trigger,
                                   GPIOInterruptCallback const & callback) {
        interrupts[gpioNumber] = callback;
        gpio_set_irq_enabled_with_callback(gpioNumber,
                                           static_cast<uint32_t>(trigger),
                                           true,
                                           &GPIOWrapper::interruptWrapper);
    }

    void GPIOWrapper::enableADC(uint8_t const gpioNumber) {
        adc_gpio_init(gpioNumber);
    }

    uint16_t GPIOWrapper::readADC(uint8_t const adcChannel) {
        adc_select_input(adcChannel);
        return adc_read();
    }

}
