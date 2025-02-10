/**
 * ProjectConfig.h
 *
 * @author Patrick Lavigne
 */
#pragma once

#include <string>
#include <cstdint>

namespace kilight::conf {
    struct project_config_data_t {
        std::string_view const ProjectName;
        std::string_view const VersionString;
        std::uint32_t const VersionMajor;
        std::uint32_t const VersionMinor;
        std::uint32_t const VersionPatch;
        std::string_view const DeviceName;
        std::string_view const ManufacturerName;
        std::string_view const HardwareVersionString;
        std::uint32_t const HardwareVersionMajor;
        std::uint32_t const HardwareVersionMinor;
        std::uint32_t const HardwareVersionPatch;
    };

    project_config_data_t const & getProjectConfig();
}
