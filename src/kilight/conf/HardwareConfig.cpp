/**
 * HardwareConfig.cpp
 *
 * @author Patrick Lavigne
 */
#include "kilight/conf/HardwareConfig.h"

#include <cstring>
#include <pico/unique_id.h>

namespace kilight::conf {
    uint64_t HardwareConfig::getUniqueID() {
        static_assert(sizeof(pico_unique_board_id_t::id) == sizeof(uint64_t),
                      "pico_unique_board_id_t id must be 64 bits exactly");
        // Board ID is cached statically in the pico library so retrieving it shouldn't add much overhead
        pico_unique_board_id_t boardId;
        pico_get_unique_board_id(&boardId);
        uint64_t buff;
        memcpy(&buff, boardId.id, sizeof(uint64_t));
        return buff;
    }
}
