/**
 * LogSink.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include "mpf/core/LogSink.h"

namespace kilight::core {

    class LogSink final : public mpf::core::LogSink {
    public:
        LogSink() = default;

        ~LogSink() override = default;

        void writeTimeStamp() const override;

        void write(std::string_view const& message) const override;

        void flush() const override;
    };
}
