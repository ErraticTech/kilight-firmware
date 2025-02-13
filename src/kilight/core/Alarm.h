/**
 * Alarm.h
 *
 * @author Patrick Lavigne
 */


#pragma once

#include <functional>
#include <cstdint>

#include <pico/time.h>

namespace kilight::core {
    class Alarm {

        static int64_t callbackWrapper(alarm_id_t const id, void * const context) {
            if (context == nullptr) {
                return 0;
            }
            auto castContext = static_cast<Alarm *>(context);
            if (id == castContext->m_activeAlarmId) {
                castContext->m_activeAlarmId = -1;
            }
            if (castContext->m_callback) {
                castContext->m_callback(*castContext);
            }
            return 0;
        }

    public:
        Alarm() noexcept = default;

        template<typename FuncT>
        void setTimeout(uint32_t const milliseconds, FuncT &&callback) {
            cancel();
            m_callback = std::forward<FuncT>(callback);
            m_activeAlarmId = add_alarm_in_ms(milliseconds,
                                              &Alarm::callbackWrapper,
                                              this,
                                              true);
        }

        template<typename FuncT>
        void setTimeoutUs(uint64_t const microseconds, FuncT &&callback) {
            cancel();
            m_callback = std::forward<FuncT>(callback);
            m_activeAlarmId = add_alarm_in_us(microseconds,
                                              &Alarm::callbackWrapper,
                                              this,
                                              true);
        }

        void restart(uint32_t const milliseconds) {
            if (m_activeAlarmId > -1) {
                cancel_alarm(m_activeAlarmId);
                m_activeAlarmId = -1;
            }
            m_activeAlarmId = add_alarm_in_ms(milliseconds,
                                              &Alarm::callbackWrapper,
                                              this,
                                              true);
        }

        void cancel() {
            if (m_activeAlarmId > -1) {
                cancel_alarm(m_activeAlarmId);
                m_callback = std::function<void(Alarm &)>();
                m_activeAlarmId = -1;
            }
        }

    private:
        alarm_id_t m_activeAlarmId = -1;

        std::function<void(Alarm &)> m_callback;
    };
}
