/**
 * ADC
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

#include <mpf/util/macros.h>
#include <mpf/core/Logging.h>

namespace kilight::hw {

    class ADC final {
        LOGGER(ADC);
    public:
        struct PACKED sample_t {
            uint16_t volatile channelZero: 16;
        };

        static constexpr size_t SampleBufferSize = 128;

        static constexpr uint16_t ReferenceVoltageMilliVolts = 3000;

        static constexpr uint16_t ADCMaxValue = 0x0FFF;

        static constexpr float MilliVoltsPerADCTick = static_cast<float>(ReferenceVoltageMilliVolts) / static_cast<float>(ADCMaxValue);

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

        static inline volatile bool dataIsReady = false;
    };

}
