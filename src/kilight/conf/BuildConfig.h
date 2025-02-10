/**
 * BuildConfig.h
 *
 * @author Patrick Lavigne
 */
#pragma once

#include <string>
#include <cstdint>

namespace kilight::conf {
    struct build_config_data_t {
        std::string_view const Compiler;
        std::string_view const GitRevision;
        std::string_view const GitShortRevision;
        std::string_view const GitBranch;
        std::string_view const Date;
        std::string_view const Time;
        std::string_view const Timestamp;
        std::uint32_t const Number;
        std::string_view const InfoString;
    };

    build_config_data_t const & getBuildConfig();
}
