/**
 * KiLight.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <atomic>

#include <mpf/core/Program.h>
#include <mpf/core/Logging.h>

#include "kilight/core/LogSink.h"
#include "kilight/com/WifiSubsystem.h"

namespace kilight {

    class KiLight final : public mpf::core::Program {
        LOGGER(KiLight);

        static constexpr uint32_t WatchdogTimeoutMs = 2000;

    public:
        KiLight();

        ~KiLight() override = default;

    protected:
        [[nodiscard]]
        core::LogSink const * logSink() const override;

        void initialize() override;

        void printStartupHeader() const override;

        void setUp() override;

        void beforeFirstLoop() override;

        void afterEveryLoop() override;

        [[noreturn]]
        void panic(char const * message) override;

    private:
        core::LogSink const m_logSink;

        com::WifiSubsystem m_wifiSubsystem;
    };

} // namespace kilight
