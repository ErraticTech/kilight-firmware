/**
 * KiLight.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/KiLight.h"

#include <hardware/watchdog.h>
#include <pico/stdlib.h>

#include "kilight/conf/BuildConfig.h"
#include "kilight/conf/ProjectConfig.h"
#include "kilight/hw/SysClock.h"

using mpf::core::Logger;
using kilight::hw::SysClock;
using kilight::core::LogSink;

namespace kilight {

    LogSink const * KiLight::logSink() const {
        return &m_logSink;
    }

    void KiLight::initialize() {
        SysClock::initialize();
        setup_default_uart();
    }

    void KiLight::setUp() {
        printStartupHeader();
    }

    void KiLight::beforeFirstLoop() {
        watchdog_enable(WatchdogTimeoutMs, KILIGHT_PAUSE_WDT_ON_DEBUG);
    }

    void KiLight::afterEveryLoop() {
        watchdog_update();
    }

    void KiLight::panic(char const * message) {
        ::panic(message);
    }

    void KiLight::printStartupHeader() {
        auto const & projectConf = conf::getProjectConfig();
        auto const & buildConf = conf::getBuildConfig();

        INFOCRLF();
        INFOCRLF();
        INFOC("\t{} v{}", projectConf.ProjectName, projectConf.VersionString);
        INFOC("\t{}", buildConf.InfoString);
        INFOCRLF();
        INFOF();
    }
} // kilight
