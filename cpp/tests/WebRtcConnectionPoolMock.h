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
//

#ifndef WebRtcConnectionPoolMock_h
#define WebRtcConnectionPoolMock_h

#include <gmock/gmock.h>

#include "WebRtcConnectionPool.h"

class WebRtcConnectionPoolMock: public WebRtcConnectionPool {
public:
    WebRtcConnectionPoolMock(SignalServerClient* client,
                             WebRtcConnectionPoolListener* listener): WebRtcConnectionPool(client, listener) {}
    ~WebRtcConnectionPoolMock() { Destructor(); }
    MOCK_METHOD0(Destructor, void());
    MOCK_METHOD0(Init, void());
    MOCK_METHOD1(AddIceServer, void(webrtc::PeerConnectionInterface::IceServer));
    MOCK_METHOD1(Connect, void(std::string));
    MOCK_METHOD1(Disconnect, void(std::string));
    MOCK_METHOD3(Send, void(std::string, const rtc::CopyOnWriteBuffer&, bool));
    MOCK_METHOD1(RequestStatistic, void(std::string));
    MOCK_METHOD0(Close, void());
    
    // SignalServerClientListener interface
    MOCK_METHOD2(on_message_from_peer, void(const char*, const char*));
    
    // WebRtcConnectionListener interface
    MOCK_METHOD1(OnConnected, void(std::string));
    MOCK_METHOD1(OnDisconnected, void(std::string));
    MOCK_METHOD2(OnMessage, void(std::string, std::string));
    MOCK_METHOD2(OnBufferedAmountChanged, void(std::string, uint64_t));
    MOCK_METHOD2(OnError, void(std::string, std::string));
    MOCK_METHOD2(OnFatalError, void(std::string, std::string));
};

#endif /* WebRtcConnectionPoolMock_h */
