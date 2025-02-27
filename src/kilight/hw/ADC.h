/**
 * ADC
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <atomic>

#include <mpf/util/macros.h>
#include <mpf/core/Logging.h>

namespace kilight::hw {

    class ADC final {
        LOGGER(ADC);
    public:
        struct PACKED adc_reading_t {
            uint16_t volatile value: 12;
            unsigned: 3;
            bool volatile error: 1;
        };

        struct PACKED sample_t {
            adc_reading_t channelZero;

            #ifdef KILIGHT_HAS_OUTPUT_B
            adc_reading_t channelOne;
            #endif
        };

        static constexpr size_t SampleBufferSize = 128;

        static constexpr uint16_t ReferenceVoltageMilliVolts = 3000;

        static constexpr uint16_t ADCMaxValue = 0x0FFF;

        static constexpr float MilliVoltsPerADCBit = static_cast<float>(ReferenceVoltageMilliVolts) / static_cast<float>(ADCMaxValue);

        static void onConversionCompleteCallback();

        static void start();

        [[nodiscard]]
        static bool conversionInProgress();

        [[nodiscard]]
        static bool dataReady();

        static void clearDataReady();

        [[nodiscard]]
        static std::array<sample_t, SampleBufferSize> const & data();

        ADC() = delete;

        ~ADC() = delete;

    private:
        static inline int dmaChannel = -1;

        static inline std::array<sample_t, SampleBufferSize> sampleBuffer = {};

        static inline std::atomic_flag adcInitialized = false;

        static inline bool volatile dataIsReady = false;

        static inline std::atomic_flag volatile conversionRunning = false;

        static void setUpADC();

        static void setUpDMA();
    };

}
