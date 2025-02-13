/**
 * server_protocol.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/com/server_protocol.h"

#include "kilight/conf/ProjectConfig.h"
#include "mpf/util/StringUtil.h"

using kilight::conf::getProjectConfig;
using mpf::util::StringUtil;

namespace kilight::com {
    system_info_t::system_info_t() {
        auto const& projectConfig = getProjectConfig();
        StringUtil::copyToFixedLengthBuffer(projectConfig.DeviceName, model, sizeof(model));
        StringUtil::copyToFixedLengthBuffer(projectConfig.ManufacturerName,
                                            manufacturer,
                                            sizeof(manufacturer));
        StringUtil::copyToFixedLengthBuffer(projectConfig.VersionString,
                                            firmwareVersion,
                                            sizeof(firmwareVersion));
        StringUtil::copyToFixedLengthBuffer(projectConfig.HardwareVersionString,
                                            hardwareVersion,
                                            sizeof(hardwareVersion));
    }
}
