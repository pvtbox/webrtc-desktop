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

#ifndef WEBRTC_CONNECTION_POOL_H_
#define WEBRTC_CONNECTION_POOL_H_
#pragma once

#include <vector>
#include <map>

#include "rtc_base/scoped_ref_ptr.h"
#include "api/peerconnectioninterface.h"

#include "webrtc_api.h"
#include "WebRtcConnectionListener.h"


namespace Json {
    class Value;
}

class WebRtcConnectionPoolListener;
class WebRtcConnectionFactory;
class WebRtcConnection;

class WebRtcConnectionPool : public WebRtcConnectionListener,
                             public rtc::RefCountInterface {
public:
    WebRtcConnectionPool(WebRtcConnectionPoolListener* listener,
                         WebRtcConnectionFactory* factory = nullptr);
    virtual void Init();
    virtual void AddIceServer(webrtc::PeerConnectionInterface::IceServer ice);
    virtual void CreateConnection(std::string connectionId);
    virtual void InitiateConnection(std::string connectionId);
    virtual void SetRemoteDescription(std::string connectionId, std::string type, std::string sdp);
    virtual void SetCandidate(std::string connectionId, std::string sdpMid, int sdpMLineIndex, std::string candidate);
    virtual void Disconnect(std::string connectionId);
    virtual void Send(std::string connectionId, const rtc::CopyOnWriteBuffer& data, bool binary);
    virtual void RequestStatistic(std::string connectionId);
    virtual void Close();

protected:
    virtual ~WebRtcConnectionPool();

    // WebRtcConnectionListener interface
    void OnConnected(std::string id) override;
    void OnDisconnected(std::string id) override;
    void OnMessage(std::string id, std::string message) override;
    void OnBufferedAmountChanged(std::string id, uint64_t amount) override;
    void OnLocalDescription(std::string id, std::string type, std::string sdp) override;
    void OnCandidate(std::string id, std::string sdpMid,int sdpMLineIndex, std::string candidate) override;
    void OnStatistic(std::string id, std::string statistic) override;

private:
    typedef rtc::scoped_refptr<WebRtcConnection> ConnectionRefPtr;
    ConnectionRefPtr FindAndEraseConnection(std::string connectionId);
    ConnectionRefPtr FindConnection(std::string connectionId);
    void HandleMessage(std::string connectionId, ConnectionRefPtr& connection, const char* message);
    void HandleSessionDescriptionMessage(std::string connectionId, ConnectionRefPtr& connection, std::string type, Json::Value* jmessage);
    void HandleCandidateMessage(ConnectionRefPtr& connection, Json::Value* jmessage);

    typedef std::map<std::string, ConnectionRefPtr> ConnectionMap;
    ConnectionMap connections_;

    ConnectionRefPtr CreateNewConnection(std::string connectionId);

    WebRtcConnectionPoolListener* listener_;

    typedef std::vector<webrtc::PeerConnectionInterface::IceServer> IceServers;
    IceServers ice_;
    
    std::unique_ptr<WebRtcConnectionFactory> factory_;
};

#endif  // WEBRTC_CONNECTION_POOL_H_
