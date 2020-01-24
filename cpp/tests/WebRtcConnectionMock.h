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

#ifndef WebRtcConnectionMock_h
#define WebRtcConnectionMock_h

#include <gmock/gmock.h>

#include "WebRtcConnection.h"

class WebRtcConnectionMock: public WebRtcConnection {
public:
    WebRtcConnectionMock() : WebRtcConnection("0", nullptr, nullptr) {}
    ~WebRtcConnectionMock() { Destructor(); }
    MOCK_METHOD0(Destructor, void());
    MOCK_METHOD2(Init, void(webrtc::PeerConnectionFactoryInterface* factory, IceServers ice));
    MOCK_METHOD2(Send, void(const rtc::CopyOnWriteBuffer&, bool));
    MOCK_METHOD0(CreateOffer, void());
    MOCK_METHOD0(CreateAnswer, void());
    MOCK_METHOD1(SetRemoteDescription, void(webrtc::SessionDescriptionInterface*));
    MOCK_METHOD1(AddIceCandidate, bool(webrtc::IceCandidateInterface*));
    MOCK_METHOD0(Close, void());
    MOCK_CONST_METHOD0(GetId, std::string());
    MOCK_METHOD0(RequestStatistic, void());
};

#endif /* WebRtcConnectionMock_h */
