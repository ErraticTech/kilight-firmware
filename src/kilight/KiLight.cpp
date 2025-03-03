/**
 * KiLight.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/KiLight.h"

#include <pico/stdlib.h>
#include <hardware/adc.h>
#include <hardware/i2c.h>
#include <hardware/watchdog.h>

#include <mpf/conf/LibraryConfig.h>
#include <mpf/conf/BuildConfig.h>

#include "kilight/conf/BuildConfig.h"
#include "kilight/conf/ProjectConfig.h"
#include "kilight/hw/SysClock.h"
#include "kilight/hw/SystemPins.h"

using kilight::core::LogSink;
using kilight::hw::SysClock;
using kilight::hw::SystemPins;

namespace kilight {

    KiLight::KiLight() :
        Program(),
        m_storageSubsystem(subsystems()),
        m_userInterfaceSubsystem(subsystems()),
        m_oneWireSubsystem(subsystems()),
        m_wifiSubsystem(subsystems(), &m_storageSubsystem, &m_userInterfaceSubsystem),
        m_lightSubsystem(subsystems(), &m_storageSubsystem, &m_wifiSubsystem),
        m_currentMonitorSubsystem(subsystems(), &m_wifiSubsystem, &m_lightSubsystem),
        m_thermalSubsystem(subsystems(), &m_oneWireSubsystem, &m_wifiSubsystem, &m_lightSubsystem) {
    }

    LogSink const* KiLight::logSink() const {
        return &m_logSink;
    }

    void KiLight::initialize() {
        SysClock::initialize();
        SystemPins::initPins();
        setup_default_uart();
        adc_init();
        i2c_init(i2c_default, KILIGHT_I2C_BAUD_RATE);
    }

    void KiLight::printStartupHeader() const {
        auto const& projectConf = conf::getProjectConfig();
        auto const& buildConf = conf::getBuildConfig();
        auto const& libraryConf = mpf::conf::getLibraryConfig();
        auto const& libraryBuildConf = mpf::conf::getBuildConfig();

        INFOCRLF();
        INFOCRLF();
        INFOC("\t{} v{}", projectConf.ProjectName, projectConf.VersionString);
        INFOC("\t{}", buildConf.InfoString);
        INFOC("\tUsing {} v{}", libraryConf.LibraryName, libraryConf.VersionString);
        INFOC("\t{}", libraryBuildConf.InfoString);
        INFOCRLF();
        INFOF();
    }

    void KiLight::setUp() {
        if (watchdog_enable_caused_reboot()) {
            WARN("Watchdog timeout caused previous reboot!");
        }
    }

    void KiLight::beforeFirstLoop() {
        watchdog_enable(WatchdogTimeoutMs, KILIGHT_PAUSE_WDT_ON_DEBUG);
    }

    void KiLight::afterEveryLoop() {
        watchdog_update();
    }

    void KiLight::panic(char const* message) {
        ::panic(message);
    }
} // kilight
