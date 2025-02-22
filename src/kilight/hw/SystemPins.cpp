/**
 * SystemPins
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/SystemPins.h"

#include <pico/stdlib.h>
#include <pico/binary_info.h>

namespace kilight::hw {

    void SystemPins::initPins() {

        OutputA::Red::initPin();
        OutputA::Green::initPin();
        OutputA::Blue::initPin();
        bi_decl(bi_3pins_with_names(OutputA::Red::Number, "Output A - Red",
                                    OutputA::Green::Number, "Output A - Green",
                                    OutputA::Blue::Number, "Output A - Blue"))

        OutputA::ColdWhite::initPin();
        OutputA::WarmWhite::initPin();
        bi_decl(bi_2pins_with_names(OutputA::ColdWhite::Number, "Output A - Cold White",
                                    OutputA::WarmWhite::Number, "Output A - Warm White"))

        OutputA::CurrentSense::initPin();
        bi_decl(bi_1pin_with_name(OutputA::CurrentSense::Number, "Output A - Current Sense"))

        FanTacho::initPin();
        FanPWM::initPin();
        bi_decl(bi_2pins_with_names(FanTacho::Number, "Fan Tachometer",
                                    FanPWM::Number, "Fan PWM"))

        ClearButton::initPin();
        bi_decl(bi_1pin_with_name(ClearButton::Number, "Clear Button"))

        DebugUARTTxD::initPin();
        DebugUARTRxD::initPin();
        bi_decl(bi_2pins_with_func(DebugUARTTxD::Number, DebugUARTRxD::Number, GPIO_FUNC_UART))

        ActivityLED::initPin();
        StatusLED::initPin();
        bi_decl(bi_2pins_with_names(ActivityLED::Number, "Activity LED", StatusLED::Number, "Status LED"))

        SDA::initPin();
        SCL::initPin();
        bi_decl(bi_2pins_with_func(SDA::Number, SCL::Number, GPIO_FUNC_I2C))

        UnusedPinA::initPin();
        UnusedPinB::initPin();
        UnusedPinC::initPin();
        UnusedPinD::initPin();
        UnusedPinE::initPin();
        UnusedPinF::initPin();
        UnusedPinG::initPin();
        UnusedPinH::initPin();
        UnusedPinI::initPin();
        UnusedPinJ::initPin();
        UnusedPinK::initPin();
    }
}
