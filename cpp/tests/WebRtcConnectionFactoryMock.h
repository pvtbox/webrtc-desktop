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

#ifndef WebRtcConnectionFactoryMock_h
#define WebRtcConnectionFactoryMock_h

#include <gmock/gmock.h>

#include "WebRtcConnectionFactory.h"

class WebRtcConnectionFactoryMock: public WebRtcConnectionFactory {
public:
    WebRtcConnectionFactoryMock() : WebRtcConnectionFactory(nullptr) {}
    ~WebRtcConnectionFactoryMock() { Destructor(); }
    MOCK_METHOD0(Destructor, void());
    MOCK_METHOD4(Create, rtc::scoped_refptr<WebRtcConnection>(std::string,
                                                              SignalServerClient*,
                                                              WebRtcConnection::IceServers,
                                                              WebRtcConnectionListener*));
};

#endif /* WebRtcConnectionFactoryMock_h */
