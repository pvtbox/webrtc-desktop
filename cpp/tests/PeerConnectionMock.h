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

#ifndef PeerConnectionMock_h
#define PeerConnectionMock_h

#include <gmock/gmock.h>

#include "webrtc/api/peerconnectioninterface.h"

class PeerConnectionMock: public webrtc::PeerConnectionInterface {
public:
    virtual ~PeerConnectionMock() { Destructor(); }
    MOCK_METHOD0(Destructor, void());
    MOCK_METHOD2(CreateOffer, void(webrtc::CreateSessionDescriptionObserver*,
                                   const webrtc::MediaConstraintsInterface*));
    MOCK_METHOD2(CreateAnswer, void(webrtc::CreateSessionDescriptionObserver*,
                                    const webrtc::MediaConstraintsInterface*));
    
    MOCK_METHOD0(local_streams, rtc::scoped_refptr<webrtc::StreamCollectionInterface>());
    MOCK_METHOD0(remote_streams, rtc::scoped_refptr<webrtc::StreamCollectionInterface>());
    MOCK_METHOD1(AddStream, bool(webrtc::MediaStreamInterface*));
    MOCK_METHOD1(RemoveStream, void(webrtc::MediaStreamInterface*));
	MOCK_METHOD2(AddTrack, rtc::scoped_refptr<webrtc::RtpSenderInterface>(webrtc::MediaStreamTrackInterface*, std::vector<webrtc::MediaStreamInterface*>));
	MOCK_METHOD1(RemoveTrack, bool(webrtc::RtpSenderInterface*));
    MOCK_METHOD1(CreateDtmfSender, rtc::scoped_refptr<webrtc::DtmfSenderInterface>(webrtc::AudioTrackInterface*));
    MOCK_METHOD3(GetStats, bool(webrtc::StatsObserver*, webrtc::MediaStreamTrackInterface*, webrtc::PeerConnectionInterface::StatsOutputLevel));
    MOCK_METHOD1(GetStats, void(webrtc::RTCStatsCollectorCallback*));
    MOCK_METHOD2(CreateDataChannel, rtc::scoped_refptr<webrtc::DataChannelInterface>(const std::string&, const webrtc::DataChannelInit*));
    MOCK_CONST_METHOD0(local_description, const webrtc::SessionDescriptionInterface*());
    MOCK_CONST_METHOD0(remote_description, const webrtc::SessionDescriptionInterface*());
    MOCK_METHOD2(SetLocalDescription, void(webrtc::SetSessionDescriptionObserver*, webrtc::SessionDescriptionInterface*));
    MOCK_METHOD2(SetRemoteDescription, void(webrtc::SetSessionDescriptionObserver*, webrtc::SessionDescriptionInterface*));
    MOCK_METHOD1(AddIceCandidate, bool(const webrtc::IceCandidateInterface*));
    MOCK_METHOD1(RegisterUMAObserver, void(webrtc::UMAObserver*));
    MOCK_METHOD0(signaling_state, SignalingState());
    MOCK_METHOD0(ice_connection_state, IceConnectionState());
    MOCK_METHOD0(ice_gathering_state, IceGatheringState());
    MOCK_METHOD0(Close, void());
};

#endif /* PeerConnectionMock_h */
