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
        conversionRunning.clear();
    }

    void ADC::start() {
        if (conversionRunning.test_and_set()) {
            return;
        }

        if (!adcInitialized.test_and_set()) {
            setUpADC();
        }

        dataIsReady = false;

        dma_channel_set_write_addr(dmaChannel, sampleBuffer.data(), true);
        adc_select_input(0);
        adc_run(true);
    }

    bool ADC::conversionInProgress() {
        return conversionRunning.test();
    }

    std::array<ADC::sample_t, ADC::SampleBufferSize> const &ADC::data() {
        return sampleBuffer;
    }

    void ADC::setUpADC() {
        adc_fifo_setup(true, true, 1, true, false);
        adc_set_clkdiv(KILIGHT_ADC_CLKDIV);

        #ifdef KILIGHT_HAS_OUTPUT_B
        adc_set_round_robin(0b0011);
        #else
        adc_set_round_robin(0b0001);
        #endif

        if (dmaChannel < 0) {
            setUpDMA();
        }
    }

    void ADC::setUpDMA() {
        dmaChannel = dma_claim_unused_channel(true);

        if (dmaChannel < 0) {
            panic("ADC could not get an available DMA channel!");
        }

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

    bool ADC::dataReady() {
        return dataIsReady;
    }

    void ADC::clearDataReady() {
        dataIsReady = false;
    }
}
