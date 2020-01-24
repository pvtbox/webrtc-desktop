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

#ifndef PeerConnectionFactoryInterfaceMock_h
#define PeerConnectionFactoryInterfaceMock_h

#include <gmock/gmock.h>

//#include "webrtc/api/peerconnectionfactory.h"
#include "webrtc/pc/peerconnectionfactory.h"

class PeerConnectionFactoryInterfaceMock: public webrtc::PeerConnectionFactory {
public:
    PeerConnectionFactoryInterfaceMock() : webrtc::PeerConnectionFactory(nullptr, nullptr) {}
    virtual ~PeerConnectionFactoryInterfaceMock() { Destructor(); }
    MOCK_METHOD0(Destructor, void());
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> CreatePeerConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& configuration,
                                                                             std::unique_ptr<cricket::PortAllocator> allocator,
                                                                             std::unique_ptr<rtc::RTCCertificateGeneratorInterface> cert_generator,
                                                                             webrtc::PeerConnectionObserver* observer) override {
        return CreatePeerConnection(configuration, observer);
    }
    MOCK_METHOD2(CreatePeerConnection,
                 rtc::scoped_refptr<webrtc::PeerConnectionInterface>(const webrtc::PeerConnectionInterface::RTCConfiguration&,
                                                                     webrtc::PeerConnectionObserver*));
    MOCK_METHOD1(SetOptions, void(const webrtc::PeerConnectionFactoryInterface::Options& options));
};

#endif /* PeerConnectionFactoryInterfaceMock_h */
