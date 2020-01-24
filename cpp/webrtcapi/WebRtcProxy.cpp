/**
*  
*  Pvtbox. Fast and secure file transfer & sync directly across your devices. 
*  Copyright © 2020  Pb Private Cloud Solutions Ltd. 
*  
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*  
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*  
**/

#include <cassert>
#include "rtc_base/ssladapter.h"
#include "rtc_base/thread.h"
#include "rtc_base/copyonwritebuffer.h"

#ifdef WEBRTC_WIN
#include "rtc_base/win32socketinit.h"
#endif
#include "api/peerconnectioninterface.h"

#include "WebRtcProxy.h"
#include "WebRtcConnectionPoolFactory.h"
#include "WebRtcConnectionPool.h"
#include "RefPtrDisposer.h"


namespace {
    class WebRtcProxyMessage : public rtc::MessageData {
    public:
        enum Action {
            Init,
            AddIceServer,
            CreateConnection,
            InitiateConnection,
            SetRemoteDescription,
            SetCandidate,
            Disconnect,
            Send,
            RequestStatistic,
            Deinit,
            OnConnected,
            OnDisconnected,
            OnMessage,
            OnBufferedAmountChanged,
            OnLocalDescription,
            OnCandidate,
            OnStatistic,
            DetachThread,
            Undefined
        };

        WebRtcProxyMessage(Action a) : action(a) {}
        WebRtcProxyMessage(Action a,
                           std::string connection_id) : action(a),
                                                     connectionId(connection_id) {}
        WebRtcProxyMessage(Action a,
                           rtc::CopyOnWriteBuffer buf,
                           bool bin,
                           std::string connection_id) : action(a),
                                                     data(buf),
                                                     binary(bin),
                                                     connectionId(connection_id) {}
        WebRtcProxyMessage(Action a,
                           webrtc::PeerConnectionInterface::IceServer ice) : action(a),
                                                                             iceServer(ice) {}
        WebRtcProxyMessage(Action a,
                           std::string connection_id,
                           std::string sdp_type,
                           std::string sdp) : action(a),
                                              connectionId(connection_id),
                                              sdp1(sdp_type),
                                              sdp2(sdp) {}
        WebRtcProxyMessage(Action a,
                           std::string connection_id,
                           std::string sdp_mid,
                           int sdp_m_line_index,
                           std::string candidate) : action(a),
                                                    connectionId(connection_id),
                                                    sdp1(sdp_mid),
                                                    sdp2(candidate),
                                                    sdpInt(sdp_m_line_index) {}
        WebRtcProxyMessage(Action a,
                           std::string connection_id,
                           std::string msg) : action(a),
                                              connectionId(connection_id),
                                              message(msg) {}
        WebRtcProxyMessage(Action a,
                           std::string connection_id,
                           uint64_t buffered_amount) : action(a),
                                                       connectionId(connection_id),
                                                       bufferedAmount(buffered_amount) {}

        Action action;
        rtc::CopyOnWriteBuffer data;
        bool binary;
        std::string connectionId;
        webrtc::PeerConnectionInterface::IceServer iceServer;
        std::string message;
        uint64_t bufferedAmount;
        std::string sdp1;
        std::string sdp2;
        int sdpInt;
    private:
        WebRtcProxyMessage() {};
    };
}


WebRtcProxy::WebRtcProxy(WebRtcConnectionPoolFactory* poolFactory) :
    ownThread_(rtc::Thread::CreateWithSocketServer()),
    pythonThread_(rtc::Thread::CreateWithSocketServer()),
    listener_(nullptr),
    poolFactory_(poolFactory == nullptr ? new WebRtcConnectionPoolFactory() : poolFactory) {
    ownThread_->SetName("WebRtc signal thread", this);
    ownThread_->Start();
    pythonThread_->SetName("WebRtc python thread", this);
    pythonThread_->Start();
    SendToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::Init));
}

WebRtcProxy::~WebRtcProxy() {
    RTC_LOG_F(LS_VERBOSE);
    close();
}

void WebRtcProxy::set_listener(WebRtcListener* listener) {
    listener_ = listener;
}

void WebRtcProxy::add_ice_server(std::string uri, std::string username, std::string password) {
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = uri;
    server.username = username;
    server.password = password;
    PostToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::AddIceServer, server));
    
}

void WebRtcProxy::create_connection(std::string connection_id) {
    SendToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::CreateConnection, connection_id));
}

void WebRtcProxy::initiate_connection(std::string connection_id) {
    PostToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::InitiateConnection, connection_id));
}

void WebRtcProxy::set_remote_description(std::string connection_id, std::string type, std::string sdp) {
    PostToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::SetRemoteDescription, connection_id, type, sdp));
}

void WebRtcProxy::set_candidate(std::string connection_id, std::string sdp_mid, int sdp_m_line_index, std::string candidate) {
    PostToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::SetCandidate, connection_id, sdp_mid, sdp_m_line_index, candidate));
}

void WebRtcProxy::disconnect(std::string connection_id) {
    RTC_LOG_F(LS_INFO) << "Sending close message before disconnect";
    PostToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::Disconnect, connection_id));
}

void WebRtcProxy::send(std::string connection_id, const char* data, size_t size, bool binary) {
    SendToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::Send,
                                           rtc::CopyOnWriteBuffer(data, size),
                                           binary,
                                           connection_id));
}

void WebRtcProxy::request_statistic(std::string connection_id) {
    PostToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::RequestStatistic, connection_id));
}

void WebRtcProxy::close() {
    assert(rtc::Thread::Current() != pythonThread_.get());
    assert(rtc::Thread::Current() != ownThread_.get());
    SendToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::Deinit));
#ifdef ANDROID
    SendToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::DetachThread));
#endif
    pythonThread_->Quit();
    pythonThread_.reset();
#ifdef ANDROID
    SendToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::DetachThread));
#endif
    ownThread_->Quit();
    ownThread_.reset();

}

#ifdef ANDROID
void WebRtcProxy::set_java_vm(JavaVM* jvm) {
    jvm_ = jvm;
}
#endif

void WebRtcProxy::PostToOwnThread(WebRtcProxyMessage* msg) {
    if(ownThread_.get()) {
        RTC_LOG_F(LS_INFO) << " " << ownThread_->size();
        ownThread_->Post(RTC_FROM_HERE, this, 0, msg);
    }
}

void WebRtcProxy::SendToOwnThread(WebRtcProxyMessage* msg) {
    if (ownThread_.get()) {
        RTC_LOG_F(LS_INFO) << " " << ownThread_->size();
        ownThread_->Send(RTC_FROM_HERE, this, 0, msg);
    }
}

void WebRtcProxy::PostToPythonThread(WebRtcProxyMessage* msg) {
    if (pythonThread_.get()) {
        RTC_LOG_F(LS_INFO) << " " << pythonThread_->size();
        pythonThread_->Post(RTC_FROM_HERE, this, 0, msg);
    }
}

void WebRtcProxy::SendToPythonThread(WebRtcProxyMessage* msg) {
    if (pythonThread_.get()) {
        RTC_LOG_F(LS_INFO) << " " << pythonThread_->size();
        pythonThread_->Send(RTC_FROM_HERE, this, 0, msg);
    }
}

// WebRtcConnectionPoolListenerInterface
void WebRtcProxy::OnConnected(std::string id) {
    PostToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::OnConnected, id));
}

void WebRtcProxy::OnDisconnected(std::string id) {
    PostToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::OnDisconnected, id));
    PostToOwnThread(new WebRtcProxyMessage(WebRtcProxyMessage::Disconnect, id));
}

void WebRtcProxy::OnMessage(std::string id, std::string message) {
    PostToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::OnMessage, id, message));
}

void WebRtcProxy::OnBufferedAmountChanged(std::string id, uint64_t amount) {
    PostToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::OnBufferedAmountChanged, id, amount));
}

void WebRtcProxy::OnLocalDescription(std::string id, std::string type, std::string sdp) {
    PostToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::OnLocalDescription, id, type, sdp));
}

void WebRtcProxy::OnCandidate(std::string id, std::string sdpMid, int sdpMLineIndex, std::string candidate) {
    PostToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::OnCandidate, id, sdpMid, sdpMLineIndex, candidate));
}

void WebRtcProxy::OnStatistic(std::string id, std::string statistic) {
    PostToPythonThread(new WebRtcProxyMessage(WebRtcProxyMessage::OnStatistic, id, statistic));
}

// rtc::MessageHandler interface
void WebRtcProxy::OnMessage(rtc::Message *msg) {
    WebRtcProxyMessage* message = reinterpret_cast<WebRtcProxyMessage*>(msg->pdata);
    switch (message->action) {
    case WebRtcProxyMessage::Init:
        RTC_LOG_F(LS_INFO) << "Init";
#ifdef _WIN32
        rtc::EnsureWinsockInit();
#endif
        rtc::InitRandom(time(nullptr));
        rtc::InitializeSSL();
        connectionPool_ = poolFactory_->Create(this);
        assert(connectionPool_);
        connectionPool_->Init();
        break;
    case WebRtcProxyMessage::AddIceServer:
        RTC_LOG_F(LS_INFO) << "AddIceServer";
        if(connectionPool_.get()) connectionPool_->AddIceServer(message->iceServer);
        break;
    case WebRtcProxyMessage::CreateConnection:
        RTC_LOG_F(LS_INFO) << "CreateConnection " << message->connectionId;
        if (connectionPool_.get()) connectionPool_->CreateConnection(message->connectionId);
        break;
    case WebRtcProxyMessage::InitiateConnection:
        RTC_LOG_F(LS_INFO) << "InitiateConnection " << message->connectionId;
        if (connectionPool_.get()) connectionPool_->InitiateConnection(message->connectionId);
        break;
    case WebRtcProxyMessage::SetRemoteDescription:
        RTC_LOG_F(LS_INFO) << "SetRemoteDescription " << message->connectionId;
        if (connectionPool_.get()) connectionPool_->SetRemoteDescription(message->connectionId,
                                                                         message->sdp1,
                                                                         message->sdp2);
        break;
    case WebRtcProxyMessage::SetCandidate:
        RTC_LOG_F(LS_INFO) << "SetCandidate " << message->connectionId;
        if (connectionPool_.get()) connectionPool_->SetCandidate(message->connectionId,
                                                                 message->sdp1,
                                                                 message->sdpInt,
                                                                 message->sdp2);
            break;
    case WebRtcProxyMessage::Disconnect:
        RTC_LOG_F(LS_INFO) << "Disconnect " << message->connectionId;
        if (connectionPool_.get()) connectionPool_->Disconnect(message->connectionId);
        break;
    case WebRtcProxyMessage::Send:
        RTC_LOG_F(LS_VERBOSE) << "Send " << message->connectionId;
        if (connectionPool_.get()) connectionPool_->Send(message->connectionId, message->data, message->binary);
        break;
    case WebRtcProxyMessage::RequestStatistic:
        RTC_LOG_F(LS_INFO) << "RequestStatistic " << message->connectionId;
        if (connectionPool_.get()) connectionPool_->RequestStatistic(message->connectionId);
        break;
    case WebRtcProxyMessage::Deinit:
        RTC_LOG_F(LS_INFO) << "Deinit";
        if (connectionPool_.get()) {
            connectionPool_->Close();
            ownThread_->Dispose(new RefPtrDisposer<WebRtcConnectionPool>(connectionPool_));
            connectionPool_ = nullptr;
            listener_ = nullptr;
        }
        rtc::CleanupSSL();
        break;
    case WebRtcProxyMessage::OnLocalDescription:
        RTC_LOG_F(LS_INFO) << "OnLocalDescription " << message->connectionId;
        if (listener_) listener_->on_local_description(message->connectionId.c_str(),
                                                       message->sdp1.c_str(),
                                                       message->sdp2.c_str());
        break;
    case WebRtcProxyMessage::OnCandidate:
        RTC_LOG_F(LS_INFO) << "OnCandidate " << message->connectionId;
        if (listener_) listener_->on_candidate(message->connectionId.c_str(),
                                               message->sdp1.c_str(),
                                               message->sdpInt,
                                               message->sdp2.c_str());
        break;
    case WebRtcProxyMessage::OnConnected:
        RTC_LOG_F(LS_INFO) << "OnConnected " << message->connectionId;
        if (listener_) listener_->on_connected(message->connectionId.c_str());
        break;
    case WebRtcProxyMessage::OnDisconnected:
        RTC_LOG_F(LS_INFO) << "OnDisconnected " << message->connectionId;
        if (listener_) listener_->on_disconnected(message->connectionId.c_str());
        break;
    case WebRtcProxyMessage::OnMessage:
        RTC_LOG_F(LS_VERBOSE) << "OnMessage " << message->connectionId;
        if (listener_) listener_->on_message(message->connectionId.c_str(),
                                             message->message.c_str(),
                                             message->message.size());
        break;
    case WebRtcProxyMessage::OnBufferedAmountChanged:
        RTC_LOG_F(LS_INFO) << "OnBufferedAmountChanged " << message->connectionId << " "
                       << message->bufferedAmount;
        if (listener_) listener_->on_buffered_amount_change(message->connectionId.c_str(),
                                                            message->bufferedAmount);
        break;
    case WebRtcProxyMessage::OnStatistic:
        RTC_LOG_F(LS_INFO) << "OnStatistic " << message->connectionId;
        if (listener_) listener_->on_statistic(message->connectionId.c_str(),
                                               message->message.c_str(),
                                               message->message.size());
        break;
    case WebRtcProxyMessage::DetachThread:
        RTC_LOG_F(LS_INFO) << "DeatchThread";
#ifdef ANDROID
        jvm_->DetachCurrentThread();
#endif
        break;
    default:
        assert(false);
        break;
    }
    delete message;
}
