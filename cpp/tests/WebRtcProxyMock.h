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

#ifndef WebRtcProxyMock_h
#define WebRtcProxyMock_h

#include <gmock/gmock.h>

#include "WebRtcProxy.h"
#include "WebRtcConnectionPoolFactoryMock.h"

class WebRtcProxyMock: public WebRtcProxy {
public:
    WebRtcProxyMock() : WebRtcProxy(nullptr, new testing::NiceMock<WebRtcConnectionPoolFactoryMock>()) {}
    ~WebRtcProxyMock() { Destructor(); }
    MOCK_METHOD0(GetSignalServerClient, SignalServerClient*());
    MOCK_METHOD1(set_listener, void(WebRtcListener*));
    MOCK_METHOD3(add_ice_server, void(std::string, std::string, std::string));
    MOCK_METHOD1(connect, void(std::string));
    MOCK_METHOD1(disconnect, void(std::string));
    MOCK_METHOD4(send, void(std::string, const char*, size_t, bool));
    MOCK_METHOD1(request_statistic, void(std::string));
    MOCK_METHOD0(Destructor, void());
    
    // SignalServerClient interface
    MOCK_METHOD1(set_listener, void(SignalServerClientListener*));
    MOCK_METHOD3(send_to_peer, void(const char*, const char*, unsigned long long));

    // SignalServerClientListener interface
    MOCK_METHOD2(on_message_from_peer, void(const char*, const char*));
    
    // WebRtcConnectionPoolListenerInterface
    MOCK_METHOD1(OnConnected, void(std::string));
    MOCK_METHOD1(OnDisconnected, void(std::string));
    MOCK_METHOD2(OnMessage, void(std::string, std::string));
    MOCK_METHOD2(OnBufferedAmountChanged, void(std::string, uint64_t));
    MOCK_METHOD2(OnError, void(std::string, std::string));
    MOCK_METHOD2(OnFatalError, void(std::string, std::string));
};

#endif /* WebRtcProxyMock_h */
