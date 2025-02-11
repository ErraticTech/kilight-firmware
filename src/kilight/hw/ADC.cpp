/**
 * ADC
 *
 * @author Patrick Lavigne
 */

#include "kilight/hw/ADC.h"

#include <pico/stdlib.h>
#include <hardware/adc.h>
#include <hardware/dma.h>

namespace kilight::hw {

    void ADC::onConversionCompleteCallback() {
        adc_run(false);
        adc_fifo_drain();
        dma_channel_acknowledge_irq0(dmaChannel);
        dataIsReady = true;
    }

    void ADC::start() {
        if (conversionInProgress()) {
            return;
        }

        dataIsReady = false;

        adc_fifo_setup(true,
                       true,
                       1,
                       false,
                       false);
        adc_set_clkdiv(2);
        adc_set_round_robin(0b0001);
        adc_select_input(0);

        if (dmaChannel < 0) {
            dmaChannel = dma_claim_unused_channel(true);

            DEBUG("Got DMA Channel {}", dmaChannel);

            dma_channel_config config = dma_channel_get_default_config(dmaChannel);

            channel_config_set_transfer_data_size(&config, DMA_SIZE_16);
            channel_config_set_read_increment(&config, false);
            channel_config_set_write_increment(&config, true);
            channel_config_set_dreq(&config, DREQ_ADC);

            dma_channel_configure(dmaChannel,
                                  &config,
                                  nullptr,
                                  &adc_hw->fifo,
                                  sampleBuffer.size() * (sizeof(sample_t) / sizeof(uint16_t)),
                                  false);

            dma_channel_set_irq0_enabled(dmaChannel, true);
            irq_set_priority(DMA_IRQ_0, 0xF2);
            irq_set_priority(ADC_IRQ_FIFO, 0xF1);
            irq_set_exclusive_handler(DMA_IRQ_0, &onConversionCompleteCallback);
            irq_set_enabled(DMA_IRQ_0, true);
        }

        dma_channel_set_write_addr(dmaChannel, sampleBuffer.data(), true);
        adc_run(true);
    }

    bool ADC::conversionInProgress() {
        if (dmaChannel < 0) {
            return false;
        }
        return dma_channel_is_busy(dmaChannel);
    }

    std::array<ADC::sample_t, ADC::SampleBufferSize> const &ADC::data() {
        return sampleBuffer;
    }

    bool ADC::dataReady() {
        return dataIsReady;
    }

    void ADC::clearDataReady() {
        dataIsReady = false;
    }
}
