/**
 * WifiSubsystem.cpp
 *
 * @author Patrick Lavigne
 */
#include "kilight/com/WifiSubsystem.h"

#include <cstring>

#include <pico/cyw43_arch.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>
#include <lwip/apps/mdns.h>

#include <mpf/util/StringUtil.h>

#include "kilight/conf/WifiConfig.h"
#include "kilight/conf/HardwareConfig.h"

using mpf::util::StringUtil;
using kilight::conf::getWifiConfig;
using kilight::conf::HardwareConfig;

namespace kilight::com {
    WifiSubsystem::WifiSubsystem(mpf::core::SubsystemList* list) :
        Subsystem(list),
        m_mdnsHardwareId(HardwareIdFormatString, HardwareConfig::getUniqueID()),
        m_hostname(HostNameFormatString, HardwareConfig::getUniqueID()) {
        instance = this;
    }

    void WifiSubsystem::setUp() {
        DEBUG("Setting up wifi stack...");

        if (cyw43_arch_init()) {
            panic("Failed to initialise cyw43_arch");
        }
        DEBUG("cyw43_arch initialized");

        cyw43_arch_enable_sta_mode();
        DEBUG("wifi interface initialized");

        cyw43_wifi_pm(&cyw43_state, CYW43_NONE_PM);

        netif_set_hostname(netif_list, m_hostname);

        mdns_resp_init();

        mdns_resp_add_netif(netif_list, m_hostname);

        mdns_resp_add_service(netif_list,
                              m_hostname,
                              "_kilight",
                              DNSSD_PROTO_TCP,
                              getWifiConfig().ListenPort,
                              [](mdns_service* const service, void* context) {
                                  auto const* const self = static_cast<WifiSubsystem*>(context);
                                  err_t const res = mdns_resp_add_service_txtitem(service,
                                      self->m_mdnsHardwareId,
                                      static_cast<uint8_t>(self->m_mdnsHardwareId.size()));
                                  LWIP_ERROR("mdns add service txt failed\n", res == ERR_OK, return);
                              },
                              this);

        m_state = State::Disconnected;
    }

    bool WifiSubsystem::hasWork() const {
        return true;
    }

    void WifiSubsystem::work() {
        cyw43_arch_poll();

        switch (m_state) {
            using enum State;
        case Disconnected:
            disconnectedState();
            break;

        case Connecting:
            connectingState();
            break;

        case Connected:
            connectedState();
            break;

        case PreIdle:
            preIdleState();
            break;

        case ProcessClientData:
            processClientDataState();
            break;

        case VerifyConnected:
            verifyConnectedState();
            break;

        default:
            break;
        }
        cyw43_arch_poll();
    }

    state_data_t const& WifiSubsystem::stateData() const {
        return m_stateData;
    }

    int32_t WifiSubsystem::rssi() {
        int32_t rssi = 0;
        cyw43_wifi_get_rssi(&cyw43_state, &rssi);
        return rssi;
    }

    void WifiSubsystem::retryConnectionWait() {
        m_stateAfterWait = State::Disconnected;
        m_state = State::Waiting;
        m_alarm.setTimeout(WifiConnectRetryMsec,
                           [this](core::Alarm const&) {
                               if (m_state == State::Waiting) {
                                   m_state = m_stateAfterWait;
                               }
                           });
    }

    bool WifiSubsystem::isClientDataPending() const {
        bool result = false;
        cyw43_arch_lwip_begin();
        for (connected_session_t const& session : m_connectedSessions) {
            if (session.inUse && (session.dataPending || session.sendLength > 0)) {
                result = true;
                break;
            }
        }
        cyw43_arch_lwip_end();

        return result;
    }

    void WifiSubsystem::disconnectedState() {
        if (int const errorCode = cyw43_arch_wifi_connect_async(getWifiConfig().SSID.data(),
                                                                getWifiConfig().Password.data(),
                                                                CYW43_AUTH_WPA2_AES_PSK)) {
            panic("Failed to start connection to wifi, error code: %d", errorCode);
        }

        DEBUG("Connecting to \"{}\"...", getWifiConfig().SSID);

        m_lastLinkStatus = INT_MAX;

        m_state = State::Connecting;
    }

    void WifiSubsystem::connectingState() {
        cyw43_arch_lwip_begin();
        int const linkStatus = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        cyw43_arch_lwip_end();

        if (linkStatus == m_lastLinkStatus) {
            // No update
            return;
        }

        m_lastLinkStatus = linkStatus;

        switch (linkStatus) {
        case CYW43_LINK_DOWN:
            DEBUG("Wifi link down");
            break;

        case CYW43_LINK_JOIN:
            DEBUG("Joining wifi in progress...");
            break;

        case CYW43_LINK_NOIP:
            DEBUG("Connected to wifi, no IP address yet, RSSI: {}", rssi());
            break;

        case CYW43_LINK_UP:
            DEBUG("Connected to wifi, IP address: {}, RSSI: {}", ip4addr_ntoa(netif_ip4_addr(netif_list)), rssi());
            mdns_resp_restart(netif_list);
            m_state = State::Connected;
            break;

        case CYW43_LINK_FAIL:
            ERROR("Unable to connect to wifi network with SSID \"{}\": Unknown failure", getWifiConfig().SSID);
            retryConnectionWait();
            break;

        case CYW43_LINK_NONET:
            ERROR("Unable to find wifi network with SSID \"{}\"", getWifiConfig().SSID);
            retryConnectionWait();
            break;

        case CYW43_LINK_BADAUTH:
            ERROR("Unable to connect to wifi network with SSID \"{}\": Authentication failure", getWifiConfig().SSID);
            retryConnectionWait();
            break;

        default:
            break;
        }
    }

    void WifiSubsystem::connectedState() {

        tcp_pcb* const pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);

        if (pcb == nullptr) {
            panic("Failed to create new TCP PCB, probably out of memory!");
        }

        if (err_t const err = tcp_bind(pcb, nullptr, getWifiConfig().ListenPort)) {
            panic("Failed to bind to TCP port %u, error %d", getWifiConfig().ListenPort, err);
        }

        m_serverPCB = tcp_listen_with_backlog(pcb, 1);
        if (m_serverPCB == nullptr) {
            panic("Failed to create server TCP PCB, probably out of memory!");
        }

        tcp_arg(m_serverPCB, this);
        tcp_accept(m_serverPCB,
                   [](void* const context, tcp_pcb* clientPCB, err_t const innerError) -> err_t {
                       if (context == nullptr) {
                           return ERR_VAL;
                       }

                       auto* const self = static_cast<WifiSubsystem*>(context);
                       return self->acceptCallback(clientPCB, innerError);
                   });

        INFO("Server listening at {}:{}", ip4addr_ntoa(netif_ip4_addr(netif_list)), getWifiConfig().ListenPort);

        m_state = State::PreIdle;
    }

    void WifiSubsystem::preIdleState() {
        using enum State;
        m_state = ProcessClientData;
        if (isClientDataPending()) {
            m_state = ProcessClientData;
        } else if (m_verifyConnectionNeeded) {
            m_state = VerifyConnected;
        } else {
            m_alarm.setTimeout(VerifyConnectionEveryMsec,
                               [this](core::Alarm const&) {
                                   m_verifyConnectionNeeded = true;
                                   if (m_state == Idle) {
                                       m_state = VerifyConnected;
                                   }
                               });
        }
    }

    void WifiSubsystem::processClientDataState() {
        m_state = State::PreIdle;

        cyw43_arch_lwip_begin();
        for (connected_session_t& session : m_connectedSessions) {
            if (session.inUse) {
                if (session.dataPending) {
                    processClientData(session);
                }
                if (session.sendLength != 0) {
                    sendResponse(session);
                }
            }
        }
        cyw43_arch_lwip_end();
    }

    void WifiSubsystem::verifyConnectedState() {
        m_verifyConnectionNeeded = false;

        cyw43_arch_lwip_begin();
        if (int const linkStatus = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
            linkStatus != CYW43_LINK_UP) {
            WARN("Wifi link went down (current state: {}), resetting for reconnect...", linkStatus);

            for (connected_session_t& session : m_connectedSessions) {
                closeSession(&session);
            }
            tcp_close(m_serverPCB);
            m_serverPCB = nullptr;

            m_state = State::Disconnected;
        } else {
            TRACE("Wifi link still up");
            m_state = State::PreIdle;
        }
        cyw43_arch_lwip_end();
    }

    void WifiSubsystem::processClientData(connected_session_t& session) const {
        session.dataPending = false;
        if (session.receiveLength == 0) {
            return;
        }

        uint8_t const messageLength = session.receiveBuffer[0];

        if (messageLength == 0) {
            session.receiveLength = 0;
            return;
        }

        if (messageLength > session.receiveLength - 1) {
            return;
        }

        DEBUG("Message received, processing...");

        switch (auto const requestType = static_cast<RequestType>(session.receiveBuffer[1])) {
            using enum RequestType;
        case ReadRequest:
            queueStateReply(session);
            break;

        case WriteRequest:
            processWrite(session);
            break;

        case SystemInfoRequest:
            queueSystemInfoReply(session);
            break;

        default:
            WARN("Invalid request type received: {:d}", static_cast<uint8_t>(requestType));
            break;
        }

        uint16_t const messageEnd = messageLength + 1;
        uint16_t const remainingBuffer = session.receiveLength - messageEnd;

        for (size_t iter = 0; iter < remainingBuffer; ++iter) {
            session.receiveBuffer[iter] = session.receiveBuffer[messageEnd + iter];
        }
        session.receiveLength = remainingBuffer;
        DEBUG("Message processing complete, {} bytes remaining to process", session.receiveLength);
        if (session.receiveLength > 0) {
            session.dataPending = true;
        }
    }

    void WifiSubsystem::queueStateReply(connected_session_t& session) const {
        if (static_cast<size_t>(BufferSize - session.sendLength) < sizeof(state_response_t)) {
            WARN("Insufficient send buffer space for state data reply");
            return;
        }
        DEBUG("Processing state request");
        state_response_t const wrappedData{m_stateData};
        memcpy(session.sendBuffer.begin() + session.sendLength, &wrappedData, sizeof(state_response_t));
        session.sendLength += sizeof(state_response_t);
    }

    void WifiSubsystem::queueSystemInfoReply(connected_session_t& session) {
        if (static_cast<size_t>(BufferSize - session.sendLength) < sizeof(system_info_response_t)) {
            WARN("Insufficient send buffer space for system info data reply");
            return;
        }
        DEBUG("Processing system info request");
        system_info_response_t wrappedSystemInfo;
        StringUtil::longLongToHex(HardwareConfig::getUniqueID(),
                                  std::span{
                                      wrappedSystemInfo.body.hardwareId,
                                      sizeof(wrappedSystemInfo.body.hardwareId)
                                  });
        memcpy(session.sendBuffer.begin() + session.sendLength, &wrappedSystemInfo, sizeof(system_info_response_t));
        session.sendLength += sizeof(system_info_response_t);
    }


    void WifiSubsystem::sendResponse(connected_session_t& session) {
        cyw43_arch_lwip_check();
        uint16_t const actuallySent = std::min(tcp_sndbuf(session.clientPCB), session.sendLength);
        if (actuallySent == 0) {
            return;
        }
        if (err_t const error = tcp_write(session.clientPCB,
                                          session.sendBuffer.begin(),
                                          actuallySent,
                                          TCP_WRITE_FLAG_COPY);
            error != ERR_OK) {
            WARN("Failed to write response to client, error {}", error);
            closeSession(&session);
            return;
        }
        DEBUG("Queued {} bytes to client", actuallySent);
        session.sendLength -= actuallySent;
    }

    void WifiSubsystem::processWrite(connected_session_t& session) const {
        if (session.receiveBuffer[0] - 1 != sizeof(write_request_t)) {
            WARN("Incorrect write message size received");
            return;
        }

        DEBUG("Processing write request");
        write_request_t writeRequest;
        memcpy(&writeRequest, session.receiveBuffer.begin() + 2, sizeof(write_request_t));

        if (m_writeRequestCallback) {
            m_writeRequestCallback(writeRequest);
        }
    }

    err_t WifiSubsystem::acceptCallback(tcp_pcb* const clientPCB, err_t const err) {
        if (err != ERR_OK || clientPCB == nullptr) {
            ERROR("Failed during TCP accept, error {}", err);
            if (clientPCB != nullptr) {
                tcp_close(clientPCB);
            }
            return ERR_VAL;
        }

        connected_session_t* session = nullptr;
        for (connected_session_t& possibleSession : m_connectedSessions) {
            if (possibleSession.inUse) {
                continue;
            }
            possibleSession.inUse = true;
            session = &possibleSession;
            break;
        }
        if (session == nullptr) {
            ERROR("Unable to complete TCP connection, all sessions are in use");
            return ERR_MEM;
        }

        DEBUG("Client {} connected", ip4addr_ntoa(&clientPCB->remote_ip));

        session->clientPCB = clientPCB;
        tcp_arg(session->clientPCB, session);

        tcp_recv(session->clientPCB,
                 [](void* context, tcp_pcb* tpcb, pbuf* const data, err_t const innerError) {
                     auto* const innerSession = static_cast<connected_session_t*>(context);
                     return instance->receiveCallback(innerSession, tpcb, data, innerError);
                 });

        tcp_sent(session->clientPCB,
                 [](void* context, tcp_pcb*, uint16_t const length) -> err_t {
                     auto const* const innerSession = static_cast<connected_session_t*>(context);
                     DEBUG("TCP server sent {}/{}", length, innerSession->sendLength);
                     return ERR_OK;
                 });

        tcp_err(session->clientPCB,
                [](void* context, err_t const innerError) {
                    auto* const innerSession = static_cast<connected_session_t*>(context);
                    ERROR("TCP session error, code {}", innerError);
                    closeSession(innerSession);
                });

        if (m_state == State::Idle) {
            m_state = State::ProcessClientData;
        }

        return ERR_OK;
    }

    err_t WifiSubsystem::receiveCallback(connected_session_t* const session,
                                         tcp_pcb* const tpcb,
                                         pbuf* const data,
                                         [[maybe_unused]] err_t const error) {
        if (data == nullptr) {
            DEBUG("Client {} disconnected", ip4addr_ntoa(&tpcb->remote_ip));
            return closeSession(session);
        }

        cyw43_arch_lwip_check();
        if (data->tot_len > 0) {
            DEBUG("TCP Server receive {}/{} err {}", data->tot_len, session->receiveLength, error);

            uint16_t const remainingBuffer = BufferSize - session->receiveLength;
            uint16_t const bytesToRead = data->tot_len > remainingBuffer ? remainingBuffer : data->tot_len;
            session->receiveLength += pbuf_copy_partial(data,
                                                        session->receiveBuffer.begin() + session->receiveLength,
                                                        bytesToRead,
                                                        0);
            tcp_recved(tpcb, bytesToRead);
        }
        pbuf_free(data);

        session->dataPending = true;
        if (m_state == State::Idle) {
            m_state = State::ProcessClientData;
        }

        return ERR_OK;
    }


    err_t WifiSubsystem::closeSession(connected_session_t* session) {
        if (session == nullptr) {
            return ERR_OK;
        }
        err_t err = ERR_OK;
        if (session->clientPCB != nullptr) {
            tcp_arg(session->clientPCB, nullptr);
            tcp_poll(session->clientPCB, nullptr, 0);
            tcp_sent(session->clientPCB, nullptr);
            tcp_recv(session->clientPCB, nullptr);
            tcp_err(session->clientPCB, nullptr);
            err = tcp_close(session->clientPCB);
            if (err != ERR_OK) {
                ERROR("Failed to close TCP connection (will abort), error {}", err);
                tcp_abort(session->clientPCB);
                err = ERR_ABRT;
            }
            session->clientPCB = nullptr;
        }

        session->receiveLength = 0;
        session->sendLength = 0;
        session->inUse = false;
        return err;
    }
}
