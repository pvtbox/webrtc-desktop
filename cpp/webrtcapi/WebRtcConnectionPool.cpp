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

#include "WebRtcConnectionPool.h"
#include "media/base/mediaengine.h"
#include "media/base/fakemediaengine.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "WebRtcConnectionPoolListener.h"
#include "RefPtrDisposer.h"
#include "WebRtcConnectionFactory.h"
#include "WebRtcConnection.h"


WebRtcConnectionPool::WebRtcConnectionPool(WebRtcConnectionPoolListener* listener,
                                           WebRtcConnectionFactory* factory)
  : listener_(listener),
    factory_(factory) {
}

WebRtcConnectionPool::~WebRtcConnectionPool() {
    RTC_LOG_F(LS_VERBOSE) << "WebRtcConnectionPool::~WebRtcConnectionPool";
}

void WebRtcConnectionPool::Init() {
    if (factory_.get() == nullptr) {
        factory_.reset(new WebRtcConnectionFactory(webrtc::CreateModularPeerConnectionFactory(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)));
    }
}

void WebRtcConnectionPool::AddIceServer(webrtc::PeerConnectionInterface::IceServer ice) {
    ice_.push_back(ice);
}

void WebRtcConnectionPool::CreateConnection(std::string connectionId) {
    CreateNewConnection(connectionId);
}

void WebRtcConnectionPool::InitiateConnection(std::string connectionId) {
    ConnectionRefPtr connection = FindConnection(connectionId);
    if (connection) connection->CreateOffer();
}

void WebRtcConnectionPool::SetRemoteDescription(std::string connectionId, std::string type, std::string sdp) {
    ConnectionRefPtr connection = FindConnection(connectionId);
    if (!connection) {
        RTC_LOG_F(LS_WARNING) << "Connection not found in SetRemoteDescription";
        return;
    }
    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));
    if (!session_description) {
        RTC_LOG_F(LS_WARNING) << "Can't parse received session description message. SdpParseError was : " << error.description;
        return;
    }
    connection->SetRemoteDescription(session_description);
}

void WebRtcConnectionPool::SetCandidate(std::string connectionId, std::string sdpMid, int sdpMLineIndex, std::string candidate) {
    ConnectionRefPtr connection = FindConnection(connectionId);
    if (!connection) {
        RTC_LOG_F(LS_WARNING) << "Connection not found in SetCandidate";
        return;
    }
    webrtc::SdpParseError error;
    webrtc::IceCandidateInterface* cand = webrtc::CreateIceCandidate(sdpMid,
                                                                      sdpMLineIndex,
                                                                      candidate,
                                                                      &error);
    if (!cand) {
        RTC_LOG_F(LS_WARNING) << "Can't parse received candidate message. SdpParseError was: " << error.description;
        return;
    }
    connection->AddIceCandidate(cand);
}

WebRtcConnectionPool::ConnectionRefPtr WebRtcConnectionPool::CreateNewConnection(std::string connectionId) {
    ConnectionRefPtr connection = factory_->Create(connectionId, ice_, this);
    assert(connection);
    connections_[connection->GetId()] = connection;
    return connection;
}

void WebRtcConnectionPool::Disconnect(std::string connectionId) {
    ConnectionRefPtr connection = FindAndEraseConnection(connectionId);
    if (connection.get() == nullptr) {
        RTC_LOG_F(LS_WARNING) << "Disconnect called with unexisting connection";
        return;
    }
    connection->Close();
    rtc::Thread::Current()->Dispose(new RefPtrDisposer<WebRtcConnection>(connection));
}

WebRtcConnectionPool::ConnectionRefPtr WebRtcConnectionPool::FindAndEraseConnection(std::string connectionId) {
    ConnectionMap::iterator it = connections_.find(connectionId);
    if (it == connections_.end()) return nullptr;
    ConnectionRefPtr connection = it->second;
    connections_.erase(it);
    return connection;
}

WebRtcConnectionPool::ConnectionRefPtr WebRtcConnectionPool::FindConnection(std::string connectionId) {
    ConnectionMap::iterator it = connections_.find(connectionId);
    if (it == connections_.end()) return nullptr;
    return it->second;
}

void WebRtcConnectionPool::Send(std::string connectionId, const rtc::CopyOnWriteBuffer& data, bool binary) {
    ConnectionRefPtr connection = FindConnection(connectionId);

    if (connection.get() == nullptr) {
        RTC_LOG_F(LS_WARNING) << "Send called with unexisting connection";
        return;
    }
    connection->Send(data, binary);
}

void WebRtcConnectionPool::RequestStatistic(std::string connectionId) {
    ConnectionRefPtr connection = FindConnection(connectionId);
    if (connection.get() == nullptr) {
        RTC_LOG_F(LS_WARNING) << "RequestStatistic called with unexisting connection";
        return;
    }
    connection->RequestStatistic();
}

void WebRtcConnectionPool::Close() {
    listener_ = nullptr;
    for (ConnectionMap::iterator it = connections_.begin(); it != connections_.end(); ++it) {
        ConnectionRefPtr connection = it->second;
        connection->Close();
        rtc::Thread::Current()->Dispose(new RefPtrDisposer<WebRtcConnection>(connection));
    }
    connections_.clear();
    factory_.reset();
}

// WebRtcConnectionListener interface
void WebRtcConnectionPool::OnConnected(std::string id) {
    if (listener_) listener_->OnConnected(id);
}

void WebRtcConnectionPool::OnDisconnected(std::string id) {
    if (listener_) listener_->OnDisconnected(id);
}

void WebRtcConnectionPool::OnMessage(std::string id, std::string message) {
    if (listener_) listener_->OnMessage(id, message);
    else RTC_LOG_F(LS_ERROR) << "Received pool message, but listener is nullptr";
}

void WebRtcConnectionPool::OnBufferedAmountChanged(std::string id, uint64_t amount) {
    if (listener_) listener_->OnBufferedAmountChanged(id, amount);
}

void WebRtcConnectionPool::OnLocalDescription(std::string id, std::string type, std::string sdp) {
    if (listener_) listener_->OnLocalDescription(id, type, sdp);
}

void WebRtcConnectionPool::OnCandidate(std::string id, std::string sdpMid, int sdpMLineIndex, std::string candidate) {
    if (listener_) listener_->OnCandidate(id, sdpMid, sdpMLineIndex, candidate);
}

void WebRtcConnectionPool::OnStatistic(std::string id, std::string statistic) {
    if (listener_) listener_->OnStatistic(id, statistic);
}
