/**
 * MathUtil
 *
 * @author Patrick Lavigne
 */

#include "kilight/util/MathUtil.h"

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
}
