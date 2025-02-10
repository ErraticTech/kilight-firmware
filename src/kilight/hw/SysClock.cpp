/**
 * SysClock.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/SysClock.h"

#include <pico/stdlib.h>
#include <hardware/clocks.h>

namespace kilight::hw {
    void SysClock::initialize() {
        set_sys_clock_pll(PLLVCOFrequency, PLLDiv1, PLLDiv2);
        clock_configure(clk_peri,
                        0,
                        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                        Frequency,
                        Frequency);
    }
}
