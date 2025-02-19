/**
 * MathUtil
 *
 * @author Patrick Lavigne
 */

#include "kilight/util/MathUtil.h"

#include <array>
#include <hardware/dma.h>

namespace kilight::util {
    static int configureCRCDMA() {
        static int const dmaChannel = dma_claim_unused_channel(true);
        static dma_channel_config config = dma_channel_get_default_config(dmaChannel);
        static uint8_t dummyDest;
        channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
        channel_config_set_read_increment(&config, true);
        channel_config_set_write_increment(&config, false);
        channel_config_set_sniff_enable(&config, true);
        dma_channel_set_config(dmaChannel, &config, false);
        dma_sniffer_set_output_reverse_enabled(false);
        dma_sniffer_set_output_invert_enabled(false);
        dma_sniffer_enable(dmaChannel, DMA_SNIFF_CTRL_CALC_VALUE_CRC16, true);
        dma_channel_set_write_addr(dmaChannel, &dummyDest, false);
        return dmaChannel;
    }

    uint16_t MathUtil::crc16(std::span<std::byte const> const data) {
        static int const dmaChannel = configureCRCDMA();

        dma_sniffer_set_data_accumulator(0xFFFF);
        dma_channel_set_read_addr(dmaChannel, data.data(), false);
        dma_channel_set_trans_count(dmaChannel, data.size(), true);
        dma_channel_wait_for_finish_blocking(dmaChannel);

        auto const crcValue = static_cast<uint16_t>(dma_sniffer_get_data_accumulator());
        return crcValue;
    }

    uint8_t MathUtil::crc8(std::span<std::byte const> const data) {
        std::byte crc {0};
        for (std::byte curByte : data) {
            for (uint8_t iter = 0; iter < 8; iter++) {
                bool const inputBit = std::to_integer<bool>(curByte & std::byte{0b1});
                bool const carryBit = std::to_integer<bool>(crc & std::byte{0b10000000});
                bool const shouldXOR = inputBit != carryBit;

                crc <<= 1;

                if (shouldXOR) {
                    crc ^= std::byte{0x31};
                }

                curByte >>= 1;
            }
        }
        return reverseBits(std::to_integer<uint8_t>(crc));
    }

    uint8_t MathUtil::reverseBits(uint8_t const data) {
        // Table generation taken from https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
        static constexpr std::array<uint8_t, 256> BitReverseTable {
            #define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
            #define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
            #define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
            R6(0), R6(2), R6(1), R6(3)
        };

        return BitReverseTable[data];
    }

}
