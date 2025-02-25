/**
 * WifiSubsystem.cpp
 *
 * @author Patrick Lavigne
 */
#include "kilight/com/WifiSubsystem.h"

#include <cstring>
#include <cassert>
#include <limits>

#include <pico/cyw43_arch.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>
#include <lwip/apps/mdns.h>

#include <mpf/util/StringUtil.h>

#include "kilight/conf/WifiConfig.h"
#include "kilight/conf/HardwareConfig.h"
#include "kilight/conf/ProjectConfig.h"

using mpf::util::StringUtil;
using kilight::conf::getWifiConfig;
using kilight::storage::StorageSubsystem;
using kilight::protocol::SystemState;
using kilight::protocol::Request;
using kilight::protocol::Response;
using kilight::protocol::SystemInfo;
using kilight::protocol::CommandResult;
using kilight::protocol::GetData;
using kilight::protocol::WriteOutput;

namespace kilight::com {
    WifiSubsystem::WifiSubsystem(mpf::core::SubsystemList* const list, StorageSubsystem* const storage) :
        Subsystem(list),
        m_storage(storage) {
        assert(m_storage != nullptr);
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

    SystemState const& WifiSubsystem::stateData() const {
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
            if (session.inUse && (session.dataPending || !session.writeBuffer.empty())) {
                result = true;
                break;
            }
        }
        cyw43_arch_lwip_end();

        return result;
    }

    void WifiSubsystem::disconnectedState() {
        StorageSubsystem::saveData().wifi.ssid.copyTo(m_ssidBuff);
        m_ssid = {m_ssidBuff.data(), StorageSubsystem::saveData().wifi.ssid.length()};

        StorageSubsystem::saveData().wifi.password.copyTo(m_passwordBuff);

        if (int const errorCode = cyw43_arch_wifi_connect_async(m_ssidBuff.data(),
                                                                m_passwordBuff.data(),
                                                                CYW43_AUTH_WPA2_AES_PSK)) {
            panic("Failed to start connection to wifi, error code: %d", errorCode);
        }

        DEBUG("Connecting to \"{}\"...", m_ssid);

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
            ERROR("Unable to connect to wifi network with SSID \"{}\": Unknown failure", m_ssid);
            retryConnectionWait();
            break;

        case CYW43_LINK_NONET:
            ERROR("Unable to find wifi network with SSID \"{}\"", m_ssid);
            retryConnectionWait();
            break;

        case CYW43_LINK_BADAUTH:
            ERROR("Unable to connect to wifi network with SSID \"{}\": Authentication failure", m_ssid);
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
                if (!session.writeBuffer.empty()) {
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
        uint8_t messageLength = 0;

        if (!session.readBuffer.peek(messageLength)) {
            return;
        }

        if (messageLength == 0) {
            session.readBuffer.clear();
            return;
        }

        if (messageLength > session.readBuffer.get_size() - 1) {
            return;
        }

        DEBUG("Message received, processing...");

        session.readBuffer.advance();

        Request request;

        if (EmbeddedProto::Error const errorCode = request.deserialize(session.readBuffer);
            errorCode != EmbeddedProto::Error::NO_ERRORS) {
            ERROR("Error parsing request: {}", static_cast<uint8_t>(errorCode));
            session.readBuffer.clear();
            return;
        }

        switch (request.get_which_request_type()) {
            using enum Request::FieldNumber;
        case GETDATA: {
            switch (request.get_getData()) {

            case GetData::SystemState:
                queueStateReply(session);
                break;

            case GetData::SystemInfo:
                queueSystemInfoReply(session);
                break;

            default:
                WARN("Invalid GetData type received: {:d}",
                    static_cast<uint8_t>(request.get_getData()));
                break;

            }

            break;
        }

        case WRITEOUTPUT:
            processWrite(session, request.get_writeOutput());
            break;

        default:
            WARN("Invalid request type received: {:d}", static_cast<uint8_t>(request.get_which_request_type()));
            break;
        }

        DEBUG("Message processing complete, {} bytes remaining to process", session.readBuffer.get_size());
        if (!session.readBuffer.empty()) {
            session.dataPending = true;
        }
    }

    void WifiSubsystem::queueStateReply(connected_session_t& session) const {
        DEBUG("Processing state request");
        Response response;
        response.set_systemState(m_stateData);
        queueReply(session, response);
    }

    void WifiSubsystem::sendResponse(connected_session_t& session) {
        cyw43_arch_lwip_check();
        tcpwnd_size_t const actuallySent = std::min(tcp_sndbuf(session.clientPCB),
                                                    static_cast<tcpwnd_size_t>(session.writeBuffer.get_size()));
        if (actuallySent == 0) {
            return;
        }
        if (err_t const error = tcp_write(session.clientPCB,
                                          session.writeBuffer.data().data(),
                                          actuallySent,
                                          TCP_WRITE_FLAG_COPY);
            error != ERR_OK) {
            WARN("Failed to write response to client, error {}", error);
            closeSession(&session);
            return;
        }
        DEBUG("Queued {} bytes to client", actuallySent);
        session.writeBuffer.remove(actuallySent);
    }

    void WifiSubsystem::processWrite(connected_session_t& session,
                                     WriteOutput const & writeRequest) const {
        DEBUG("Processing write request");
        Response response;
        if (m_writeRequestCallback) {
            response.set_commandResult(m_writeRequestCallback(writeRequest));
        } else {
            response.mutable_commandResult().set_result(CommandResult::Result::OK);
        }
        queueReply(session, response);
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
                     DEBUG("TCP server sent {}/{}", length, innerSession->writeBuffer.get_size());
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
            DEBUG("TCP Server receive {}/{} err {}", data->tot_len, session->readBuffer.get_size(), error);

            auto const remainingBuffer = static_cast<uint16_t>(session->readBuffer.get_available_size());
            uint16_t const bytesToRead = data->tot_len > remainingBuffer ? remainingBuffer : data->tot_len;
            if (bytesToRead > 0) {
                uint8_t buffer[BufferSize] {};
                pbuf_copy_partial(data, buffer, bytesToRead, 0);
                session->readBuffer.write(std::span(buffer, bytesToRead));
            }
            tcp_recved(tpcb, bytesToRead);
        }
        pbuf_free(data);

        session->dataPending = true;
        if (m_state == State::Idle) {
            m_state = State::ProcessClientData;
        }

        return ERR_OK;
    }


    SystemInfo WifiSubsystem::buildSystemInfo() {
        auto const& projectConfig = conf::getProjectConfig();
        std::array<char, 16> hardwareId{};
        std::format_to_n(hardwareId.begin(),
                         hardwareId.size(),
                         "{:016X}",
                         conf::HardwareConfig::getUniqueID());

        SystemInfo systemInfo;

        systemInfo.mutable_hardwareId().set(hardwareId.data(), hardwareId.size());
        systemInfo.mutable_model().set(projectConfig.DeviceName.data(), projectConfig.DeviceName.size());
        systemInfo.mutable_manufacturer().set(projectConfig.ManufacturerName.data(),
                                                      projectConfig.ManufacturerName.size());

        systemInfo.mutable_firmwareVersion().set_major(projectConfig.VersionMajor);
        systemInfo.mutable_firmwareVersion().set_minor(projectConfig.VersionMinor);
        systemInfo.mutable_firmwareVersion().set_patch(projectConfig.VersionPatch);

        systemInfo.mutable_hardwareVersion().set_major(projectConfig.HardwareVersionMajor);
        systemInfo.mutable_hardwareVersion().set_minor(projectConfig.HardwareVersionMinor);
        systemInfo.mutable_hardwareVersion().set_patch(projectConfig.HardwareVersionPatch);

        return systemInfo;
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

        session->writeBuffer.clear();
        session->readBuffer.clear();
        session->inUse = false;
        return err;
    }

    void WifiSubsystem::queueReply(connected_session_t& session, Response const& response) {
        uint32_t const size = response.serialized_size();

        assert(size < std::numeric_limits<uint8_t>::max());

        if (session.writeBuffer.get_available_size() < size + 1) {
            WARN("Insufficient send buffer space for reply (need {} bytes, have {} bytes)",
                 size + 1,
                 session.writeBuffer.get_available_size());
            return;
        }
        session.writeBuffer.push(static_cast<uint8_t>(size));

        if (EmbeddedProto::Error const errorCode = response.serialize(session.writeBuffer);
            errorCode != EmbeddedProto::Error::NO_ERRORS) {
            ERROR("Error serializing response: {}", static_cast<uint8_t>(errorCode));
            session.writeBuffer.clear();
        }
    }

    void WifiSubsystem::queueSystemInfoReply(connected_session_t& session) {
        // Since it's a somewhat heavy operation and the data never changes,
        // just store it statically after the first time
        static SystemInfo const systemInfoResponse = buildSystemInfo();
        DEBUG("Processing system info request");
        Response response;
        response.set_systemInfo(systemInfoResponse);
        queueReply(session, response);
    }
}
