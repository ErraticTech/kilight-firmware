/**
 * ServerWriteBuffer.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <span>

#include <WriteBufferInterface.h>

namespace kilight::com {

    template <uint32_t BufferSize>
    class ServerWriteBuffer final : public EmbeddedProto::WriteBufferInterface {
    public:
        void clear() override {
            m_pos = 0;
        }

        [[nodiscard]]
        uint32_t get_size() const override {
            return m_pos;
        }

        [[nodiscard]]
        uint32_t get_max_size() const override {
            return BufferSize;
        }

        [[nodiscard]]
        uint32_t get_available_size() const override {
            return BufferSize - m_pos;
        }

        bool push(uint8_t const byte) override {
            if (m_pos >= BufferSize) {
                return false;
            }
            m_data[m_pos] = byte;
            ++m_pos;
            return true;
        }

        bool push(uint8_t const* bytes, uint32_t const length) override {
            assert(bytes != nullptr);
            if (get_available_size() < length) {
                return false;
            }
            memcpy(m_data.data() + m_pos, bytes, length);
            m_pos += length;
            return true;
        }

        bool remove(uint32_t const length) {
            if (length > m_pos) {
                return false;
            }
            if (length == m_pos) {
                m_pos = 0;
                return true;
            }

            for (uint32_t iter = length; iter < m_pos; ++iter) {
                m_data[iter - length] = m_data[iter];
            }
            m_pos -= length;
            return true;
        }

        [[nodiscard]]
        std::span<uint8_t const> data() const {
            return std::span<uint8_t const>{m_data.begin(), m_pos};
        }

        [[nodiscard]]
        bool empty() const {
            return m_pos == 0;
        }

    private:
        std::array<uint8_t, BufferSize> m_data{};

        uint32_t m_pos = 0;
    };

}
