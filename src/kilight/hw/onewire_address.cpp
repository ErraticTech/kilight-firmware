/**
 * onewire_address.cpp
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/onewire_address.h"

#include <cstddef>
#include <bit>
#include <array>
#include <span>

#include "kilight/util/MathUtil.h"

using std::bit_cast;
using std::byte;
using std::array;
using std::span;

using kilight::util::MathUtil;

namespace kilight::hw {
    uint8_t onewire_address_t::calculateCrc() const {
        auto const asArray = bit_cast<array<byte, sizeof(uint64_t)>>(value());
        return MathUtil::crc8(span{asArray.begin(), asArray.size() - 1});
    }
}
