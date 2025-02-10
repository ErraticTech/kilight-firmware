/**
 * KiLight.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <atomic>

#include "mpf/core/Program.h"
#include "mpf/core/Logging.h"
#include "kilight/core/LogSink.h"

namespace kilight {

    class KiLight final : public mpf::core::Program {
        LOGGER(KiLight);

        static constexpr uint32_t WatchdogTimeoutMs = 2000;

    public:
        KiLight() = default;

        ~KiLight() override = default;

    protected:
        [[nodiscard]]
        core::LogSink const * logSink() const override;

        void initialize() override;

        void setUp() override;

        void beforeFirstLoop() override;

        void afterEveryLoop() override;

        [[noreturn]]
        void panic(char const * message) override;

    private:
        static void printStartupHeader();

        core::LogSink const m_logSink;
    };

} // namespace kilight
