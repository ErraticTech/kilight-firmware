/**
 * WifiSubsystem.h
 *
 * @author Patrick Lavigne
 */

#pragma once

#include <format>

#include <lwip/tcp.h>
#include <pico/cyw43_arch.h>


#include <mpf/types/FixedFormattedString.h>
#include <mpf/core/Logging.h>
#include <mpf/core/Subsystem.h>

#include <kilight/protocol/Request.h>
#include <kilight/protocol/Response.h>
#include <kilight/protocol/OutputIdentifier.h>

#include "kilight/com/ServerReadBuffer.h"
#include "kilight/conf/HardwareConfig.h"
#include "kilight/core/Alarm.h"
#include "kilight/storage/StorageSubsystem.h"
#include "kilight/com/ServerWriteBuffer.h"
#include "kilight/ui/UserInterfaceSubsystem.h"

#define KILIGHT_FIXED_STRING_BUFFER(name, templateString) \
        char name[sizeof(templateString)] { templateString };

namespace kilight::com {

    class WifiSubsystem final : public mpf::core::Subsystem {
        LOGGER(Wifi);

    public:
        static constexpr uint32_t WifiConnectRetryMsec = 5000;

        static constexpr uint32_t VerifyConnectionEveryMsec = 1000;

        static constexpr uint16_t BufferSize = 2048;

        static constexpr size_t MaxConnections = 8;

        static constexpr std::format_string<uint64_t> HardwareIdFormatString = "hwid={:016X}";

        static constexpr std::format_string<uint64_t> HostNameFormatString = "KiLightMono_{:016X}";

        [[nodiscard]]
        static int32_t rssi();

        [[nodiscard]]
        static char const * ipAddress();

        WifiSubsystem(mpf::core::SubsystemList* list,
                      storage::StorageSubsystem* storage,
                      ui::UserInterfaceSubsystem* ui);

        void setUp() override;

        [[nodiscard]]
        bool hasWork() const override;

        void work() override;

        [[nodiscard]]
        protocol::SystemState const& stateData() const;

        template <typename UpdateFuncT>
        void updateStateData(UpdateFuncT&& updateFunc) {
            cyw43_arch_lwip_begin();
            updateFunc(m_stateData);
            cyw43_arch_lwip_end();
        }

        template <typename UpdateFuncT>
        void updateOutputStateData(protocol::OutputIdentifier const outputId, UpdateFuncT&& updateFunc) {
            cyw43_arch_lwip_begin();
            if (outputId == protocol::OutputIdentifier::OutputA) {
                updateFunc(m_stateData.mutable_outputA());
            }
            if (outputId == protocol::OutputIdentifier::OutputB) {
                updateFunc(m_stateData.mutable_outputB());
            }
            cyw43_arch_lwip_end();
        }

        template <typename CallbackT>
        void setWriteRequestCallback(CallbackT&& callback) {
            m_writeRequestCallback = std::forward<CallbackT>(callback);
        }

    private:
        enum class State {
            Invalid,
            Disconnected,
            Connecting,
            Connected,
            PreIdle,
            Idle,
            ProcessClientData,
            VerifyConnected,
            Waiting
        };

        struct connected_session_t {
            tcp_pcb* clientPCB = nullptr;

            ServerReadBuffer<BufferSize> readBuffer{};

            ServerWriteBuffer<BufferSize> writeBuffer{};

            bool volatile inUse = false;

            bool volatile dataPending = false;
        };

        static inline WifiSubsystem* instance = nullptr;

        static protocol::SystemInfo buildSystemInfo();

        static err_t closeSession(connected_session_t* session);

        static void queueReply(connected_session_t& session, protocol::Response const& response);

        static void queueSystemInfoReply(connected_session_t& session);


        static void sendResponse(connected_session_t& session);

        State volatile m_state = State::Invalid;

        State m_stateAfterWait = State::Invalid;

        storage::StorageSubsystem* const m_storage;

        ui::UserInterfaceSubsystem* const m_ui;

        std::array<char, MaxSSIDLength + 1> m_ssidBuff{};

        std::string_view m_ssid{};

        std::array<char, MaxPasswordLength + 1> m_passwordBuff{};

        int m_lastLinkStatus = 0;

        core::Alarm m_alarm;

        tcp_pcb* m_serverPCB = nullptr;

        std::array<connected_session_t, MaxConnections> m_connectedSessions = {};

        protocol::SystemState m_stateData;

        std::function<protocol::CommandResult(protocol::WriteOutput const&)> m_writeRequestCallback;

        bool volatile m_verifyConnectionNeeded = false;

        mpf::types::FixedFormattedString<32> m_mdnsHardwareId{
                HardwareIdFormatString,
                conf::HardwareConfig::getUniqueID()
            };

        mpf::types::FixedFormattedString<32> m_hostname{
                HostNameFormatString,
                conf::HardwareConfig::getUniqueID()
            };

        void retryConnectionWait();

        [[nodiscard]]
        bool isClientDataPending() const;

        void disconnectedState();

        void connectingState();

        void connectedState();

        void preIdleState();

        void processClientDataState();

        void verifyConnectedState();

        void processClientData(connected_session_t& session) const;

        void queueStateReply(connected_session_t& session) const;

        void processWrite(connected_session_t& session, protocol::WriteOutput const& writeRequest) const;

        err_t acceptCallback(tcp_pcb* clientPCB, err_t error);

        err_t receiveCallback(connected_session_t* session, tcp_pcb* tpcb, pbuf* data, err_t error);

    };

}
