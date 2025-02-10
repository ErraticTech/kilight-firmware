/**
 * LogSink.cpp
 */

#include <cstdio>
#include <pico/stdlib.h>

#include "kilight/core/LogSink.h"
#include "mpf/util/StringUtil.h"

using std::string_view;
using mpf::util::StringUtil;

namespace kilight::core {

    void LogSink::writeTimeStamp() const {
        write(StringUtil::formatTicks(time_us_64()));
    }

    void LogSink::write(string_view const & message) const {
        fwrite(message.data(), sizeof(string_view::value_type), message.length(), stdout);
    }

    void LogSink::flush() const {
        stdio_flush();
    }
}
