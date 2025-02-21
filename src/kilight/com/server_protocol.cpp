/**
 * server_protocol.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/com/server_protocol.h"

#include "kilight/conf/ProjectConfig.h"
#include "kilight/conf/HardwareConfig.h"

using kilight::conf::getProjectConfig;
using kilight::conf::HardwareConfig;

namespace kilight::com {
    system_info_t::system_info_t() :
        hardwareId("{:016X}", HardwareConfig::getUniqueID()),
        model(getProjectConfig().DeviceName),
        manufacturer(getProjectConfig().ManufacturerName),
        firmwareVersion(getProjectConfig().VersionString),
        hardwareVersion(getProjectConfig().HardwareVersionString)
        {}
}
