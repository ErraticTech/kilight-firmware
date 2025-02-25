/**
 * ServerReadBuffer.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <span>

#include <ReadBufferInterface.h>

namespace kilight::com {

    template <uint32_t BufferSize>
    class ServerReadBuffer final : public EmbeddedProto::ReadBufferInterface {
    public:
        [[nodiscard]]
        uint32_t get_size() const override {
            return m_size;
        }

        [[nodiscard]]
        uint32_t get_max_size() const override {
            return BufferSize;
        }

        bool peek(uint8_t& byte) const override {
            if (empty()) {
                return false;
            }
            byte = m_data[m_back];
            return true;
        }

        bool advance() override {
            return advance(1);
        }

        bool advance(uint32_t const n_bytes) override {
            if (m_size < n_bytes) {
                return false;
            }
            m_back = (m_back + n_bytes) % BufferSize;
            m_size -= n_bytes;
            return true;
        }

        bool pop(uint8_t& byte) override {
            if (empty()) {
                return false;
            }
            byte = m_data[m_back];
            m_back = (m_back + 1) % BufferSize;
            m_size -= 1;
            return true;
        }

        [[nodiscard]]
        bool empty() const {
            return m_front == m_back;
        }

        void clear() {
            m_front = 0;
            m_back = 0;
        }

        [[nodiscard]]
        uint32_t get_available_size() const {
            return BufferSize - get_size();
        }

        bool write(uint8_t const byte) {
            if (get_available_size() < 1) {
                return false;
            }
            m_data[m_front] = byte;
            m_front = (m_front + 1) % BufferSize;
            m_size += 1;
            return true;
        }

        bool write(std::span<uint8_t> const & dataToWrite) {
            if (get_available_size() < dataToWrite.size()) {
                return false;
            }
            for (uint8_t const currentByte : dataToWrite) {
                m_data[m_front] = currentByte;
                m_front = (m_front + 1) % BufferSize;
            }
            m_size += dataToWrite.size();
            return true;
        }

    private:
        std::array<uint8_t, BufferSize> m_data {};

        uint32_t m_front = 0;

        uint32_t m_back = 0;

        uint32_t m_size = 0;
    };

}
