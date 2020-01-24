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
/*
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "WebRtcConnection.h"
#include "WebRtcConnectionListenerMock.h"
#include "SignalServerClientMock.h"
#include "PeerConnectionFactoryInterfaceMock.h"
#include "PeerConnectionMock.h"
#include "WebRtcChannelFactoryMock.h"
#include "WebRtcChannelMock.h"
#include "DataChannelInterfaceMock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::NotNull;
using ::testing::InvokeWithoutArgs;
using ::testing::AtLeast;

class WebRtcConnectionTest: public ::testing::Test {
protected:
    void SetUp() {
        rtc::LogMessage::LogToDebug(rtc::LS_NONE);
        factory_ = new rtc::RefCountedObject<PeerConnectionFactoryInterfaceMock>();
        pc_ = new rtc::RefCountedObject<PeerConnectionMock>();
        channelFactory_ = new WebRtcChannelFactoryMock();
        channel_ = new rtc::RefCountedObject<WebRtcChannelMock>();
        connection_ = new WebRtcConnection(std::string("101"), &client_, &listener_, channelFactory_);
        dc_ = new rtc::RefCountedObject<DataChannelInterfaceMock>();
    }
    
    void TearDown() {
        EXPECT_CALL(*(pc_.get()), Destructor())
        .Times(1);
        EXPECT_CALL(*(channel_.get()), Destructor())
        .Times(1);
        EXPECT_CALL(*channelFactory_, Destructor())
        .Times(1);
        EXPECT_CALL(*(factory_.get()), Destructor())
        .Times(1);
        EXPECT_CALL(*((DataChannelInterfaceMock*)dc_.get()), Destructor())
        .Times(1);
        connection_->Close();
        pc_ = nullptr;
        channel_ = nullptr;
        connection_ = nullptr;
        factory_ = nullptr;
        dc_ = nullptr;
        rtc::Thread::Current()->ProcessMessages(1);
    }

    rtc::scoped_refptr<WebRtcConnection> connection_;
    WebRtcConnectionListenerMock listener_;
    SignalServerClientMock client_;
    rtc::scoped_refptr<PeerConnectionFactoryInterfaceMock> factory_;
    rtc::scoped_refptr<PeerConnectionMock> pc_;
    rtc::scoped_refptr<WebRtcChannelMock> channel_;
    WebRtcChannelFactoryMock* channelFactory_;
    rtc::scoped_refptr<webrtc::DataChannelInterface> dc_;
    
public:
    void Init();
    void InitAndCreateOffer();
};

void WebRtcConnectionTest::Init() {
    EXPECT_CALL(*(factory_.get()), CreatePeerConnection(_, connection_.get()))
    .Times(1)
    .WillOnce(Return(pc_));
    connection_->Init(factory_.get(), WebRtcConnection::IceServers());
}

void WebRtcConnectionTest::InitAndCreateOffer() {
    Init();
    EXPECT_CALL(*(channel_.get()), Close())
    .Times(AtLeast(4));
    EXPECT_CALL(*channelFactory_, Create(pc_.get(), connection_.get()))
    .Times(4)
    .WillRepeatedly(Return(channel_));
    EXPECT_CALL(*(pc_.get()), CreateOffer(connection_.get(), nullptr))
    .Times(1);
    
    connection_->CreateOffer();
}

MATCHER_P(MatchIce, expected, "") {
    WebRtcConnection::IceServers actual = arg.servers;
    if (expected.size() != actual.size()) return false;
    
    for(int i = 0; i < actual.size(); i++) {
        webrtc::PeerConnectionInterface::IceServer expectedServer = expected[i];
        webrtc::PeerConnectionInterface::IceServer actualServer = actual[i];
        if (expectedServer.uri != actualServer.uri ||
            expectedServer.username != actualServer.username ||
            expectedServer.password != actualServer.password ||
            expectedServer.urls != actualServer.urls) {
            return false;
        }
    }
    return true;
}

TEST_F(WebRtcConnectionTest, GetId) {
    ASSERT_EQ(connection_->GetId(), "101");
}

TEST_F(WebRtcConnectionTest, InitFailed) {
    EXPECT_CALL(*(factory_.get()), CreatePeerConnection(_, connection_.get()))
    .Times(1)
    .WillOnce(Return(nullptr));
    EXPECT_CALL(listener_, OnFatalError(StrEq("101"), _))
    .Times(1);
    connection_->Init(factory_.get(), WebRtcConnection::IceServers());
}

TEST_F(WebRtcConnectionTest, InitOK) {
    Init();
}

TEST_F(WebRtcConnectionTest, InitCheckIce) {
    EXPECT_CALL(*(factory_.get()), CreatePeerConnection(MatchIce(WebRtcConnection::IceServers()), connection_.get()))
    .Times(1)
    .WillOnce(Return(pc_));
    connection_->Init(factory_.get(), WebRtcConnection::IceServers());
    
    WebRtcConnection::IceServers ice_;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "111";
    ice_.push_back(server);
    server.uri = "222";
    server.username = "333";
    ice_.push_back(server);
    server.uri = "444";
    server.username = "555";
    server.password = "666";
    ice_.push_back(server);
    EXPECT_CALL(*(factory_.get()), CreatePeerConnection(MatchIce(ice_), connection_.get()))
    .Times(1)
    .WillOnce(Return(pc_));
    
    connection_->Init(factory_.get(), ice_);
}

TEST_F(WebRtcConnectionTest, CreateOfferWithoutConnection) {
    EXPECT_CALL(listener_, OnFatalError(StrEq("101"), _))
    .Times(1);
    connection_->CreateOffer();
}

TEST_F(WebRtcConnectionTest, CreateOffer) {
    InitAndCreateOffer();
}

TEST_F(WebRtcConnectionTest, ChannelsPerConnectionCount) {
    Init();
    EXPECT_CALL(*(channel_.get()), Close())
    .Times(4)
    .RetiresOnSaturation();
    EXPECT_CALL(*channelFactory_, Create(pc_.get(), connection_.get()))
    .Times(4)
    .WillRepeatedly(Return(channel_))
    .RetiresOnSaturation();
    EXPECT_CALL(*(pc_.get()), CreateOffer(connection_.get(), nullptr))
    .Times(1)
    .RetiresOnSaturation();
    connection_->CreateOffer();
    
    WebRtc::set_connection_channels_count(10);
    Init();
    EXPECT_CALL(*(channel_.get()), Close())
    .Times(10)
    .RetiresOnSaturation();
    EXPECT_CALL(*channelFactory_, Create(pc_.get(), connection_.get()))
    .Times(10)
    .WillRepeatedly(Return(channel_))
    .RetiresOnSaturation();
    EXPECT_CALL(*(pc_.get()), CreateOffer(connection_.get(), nullptr))
    .Times(1)
    .RetiresOnSaturation();
    connection_->CreateOffer();
    
    WebRtc::set_connection_channels_count(1);
    Init();
    EXPECT_CALL(*(channel_.get()), Close())
    .Times(1)
    .RetiresOnSaturation();
    EXPECT_CALL(*channelFactory_, Create(pc_.get(), connection_.get()))
    .Times(1)
    .WillRepeatedly(Return(channel_))
    .RetiresOnSaturation();
    EXPECT_CALL(*(pc_.get()), CreateOffer(connection_.get(), nullptr))
    .Times(1)
    .RetiresOnSaturation();
    connection_->CreateOffer();
    
     WebRtc::set_connection_channels_count(4);
}

TEST_F(WebRtcConnectionTest, CreateAnswerWithoutConnection) {
    EXPECT_CALL(listener_, OnFatalError(StrEq("101"), _))
    .Times(1);
    connection_->CreateAnswer();
}

TEST_F(WebRtcConnectionTest, CreateAnswer) {
    Init();
    
    EXPECT_CALL(*(pc_.get()), CreateAnswer(connection_.get(), nullptr))
    .Times(1);
    
    connection_->CreateAnswer();
}

TEST_F(WebRtcConnectionTest, SetRemoteDescriptionWithoutConnection) {
    EXPECT_CALL(listener_, OnFatalError(StrEq("101"), _))
    .Times(1);
    int i = 1;
    connection_->SetRemoteDescription(reinterpret_cast<webrtc::SessionDescriptionInterface*>(&i));
}

TEST_F(WebRtcConnectionTest, SetRemoteDescription) {
    Init();
    int i = 1;
    
    EXPECT_CALL(*(pc_.get()), SetRemoteDescription(connection_.get(),
                                                   reinterpret_cast<webrtc::SessionDescriptionInterface*>(&i)))
    .Times(1);
    
    connection_->SetRemoteDescription(reinterpret_cast<webrtc::SessionDescriptionInterface*>(&i));
}

TEST_F(WebRtcConnectionTest, AddIceCandidateWithoutConnection) {
    EXPECT_CALL(listener_, OnFatalError(StrEq("101"), _))
    .Times(1);
    int i = 1;
    connection_->AddIceCandidate(reinterpret_cast<webrtc::IceCandidateInterface*>(&i));
}

TEST_F(WebRtcConnectionTest, AddIceCandidateAnswer) {
    Init();
    int i = 1;
    
    EXPECT_CALL(*(pc_.get()), AddIceCandidate(reinterpret_cast<webrtc::IceCandidateInterface*>(&i)))
    .Times(1);
    
    connection_->AddIceCandidate(reinterpret_cast<webrtc::IceCandidateInterface*>(&i));
}

TEST_F(WebRtcConnectionTest, CloseUninit) {
    connection_->Close();
}

TEST_F(WebRtcConnectionTest, CloseAfterInit) {
    Init();
    connection_->Close();
}

TEST_F(WebRtcConnectionTest, CloseAfterInitAndOffer) {
    InitAndCreateOffer();
    connection_->Close();
}

TEST_F(WebRtcConnectionTest, SendWithoutChannel) {
    EXPECT_CALL(listener_, OnError(StrEq("101"), _))
    .Times(2);
    connection_->Send(rtc::CopyOnWriteBuffer(), true);
    connection_->Send(rtc::CopyOnWriteBuffer(), false);
}

TEST_F(WebRtcConnectionTest, SendAfterInitAndOffer) {
    InitAndCreateOffer();
    EXPECT_CALL(*(channel_.get()), IsOpen())
    .Times(2)
    .WillRepeatedly(Return(true));
    EXPECT_CALL(*(channel_.get()), Send(rtc::CopyOnWriteBuffer(), true))
    .Times(1);
    EXPECT_CALL(*(channel_.get()), Send(rtc::CopyOnWriteBuffer(), false))
    .Times(1);
    connection_->Send(rtc::CopyOnWriteBuffer(), true);
    connection_->Send(rtc::CopyOnWriteBuffer(), false);
}

TEST_F(WebRtcConnectionTest, SendAfterInitAndOfferWhen3ConnectionsClosed) {
    InitAndCreateOffer();
    EXPECT_CALL(*(channel_.get()), IsOpen())
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();
    EXPECT_CALL(*(channel_.get()), IsOpen())
    .Times(3)
    .WillRepeatedly(Return(false))
    .RetiresOnSaturation();
    EXPECT_CALL(*(channel_.get()), IsOpen())
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();
    EXPECT_CALL(*(channel_.get()), IsOpen())
    .Times(3)
    .WillRepeatedly(Return(false))
    .RetiresOnSaturation();

    EXPECT_CALL(*(channel_.get()), Send(rtc::CopyOnWriteBuffer(), true))
    .Times(1);
    EXPECT_CALL(*(channel_.get()), Send(rtc::CopyOnWriteBuffer(), false))
    .Times(1);
    connection_->Send(rtc::CopyOnWriteBuffer(), true);
    connection_->Send(rtc::CopyOnWriteBuffer(), false);
}

TEST_F(WebRtcConnectionTest, RequestStatistic) {
    EXPECT_CALL(listener_, OnError(StrEq("101"), _))
    .Times(1);
    connection_->RequestStatistic();
}

TEST_F(WebRtcConnectionTest, RequestStatisticAfterInit) {
    Init();
    EXPECT_CALL(*(pc_.get()), GetStats(connection_.get()))
    .Times(1);
    connection_->RequestStatistic();
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnDataChannel) {
    EXPECT_CALL(*(channel_.get()), Close())
    .Times(1);
    EXPECT_CALL(*channelFactory_, Create(dc_, connection_.get()))
    .Times(1)
    .WillOnce(Return(channel_));
    
    ((webrtc::PeerConnectionObserver*)connection_.get())->OnDataChannel(dc_);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnDataChannelAfterInitAndOffer) {
    InitAndCreateOffer();

    EXPECT_CALL(*channelFactory_, Create(dc_, connection_.get()))
    .Times(1)
    .WillOnce(Return(channel_));
    
    ((webrtc::PeerConnectionObserver*)connection_.get())->OnDataChannel(dc_);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnDataChannelAfterOnDataChannel) {
    EXPECT_CALL(*(channel_.get()), Close())
    .Times(2);
    EXPECT_CALL(*channelFactory_, Create(dc_, connection_.get()))
    .Times(2)
    .WillRepeatedly(Return(channel_));
    ((webrtc::PeerConnectionObserver*)connection_.get())->OnDataChannel(dc_);

    ((webrtc::PeerConnectionObserver*)connection_.get())->OnDataChannel(dc_);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeNew) {
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeChecking) {
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeConnected) {
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeCompleted) {
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeFailed) {
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeDisconnected) {
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeDisconnectedAfterOnClosed) {
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnClosed();
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeClosed) {
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeMax) {
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionMax);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionChangeClosedAfterFailedAfterDisconnectedAfterConnected) {
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionReceivingChangeTrue) {
    ((webrtc::PeerConnectionObserver*)connection_.get())->OnIceConnectionReceivingChange(true);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceConnectionReceivingChangeFalse) {
    ((webrtc::PeerConnectionObserver*)connection_.get())->OnIceConnectionReceivingChange(false);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceCandidateNull) {
    ((webrtc::PeerConnectionObserver*)connection_.get())->OnIceCandidate(nullptr);
}

TEST_F(WebRtcConnectionTest, PeerConnectionObserverOnIceCandidate) {
    std::unique_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate("data",
                                                                                        0,
                                                                                        "candidate:777009151 1 udp 2122260223 192.168.20.5 55807 typ host generation 0 ufrag 4cY7 network-id 1 network-cost 50",
                                                                                        nullptr));
    EXPECT_CALL(client_,
                send_to_peer(StrEq("101"),
                             StrEq("{\n   \"candidate\" : \"candidate:777009151 1 udp 2122260223 192.168.20.5 55807 typ host generation 0 ufrag 4cY7 network-id 1 network-cost 50\",\n   \"sdpMLineIndex\" : 0,\n   \"sdpMid\" : \"data\"\n}\n"),
                             187))
    .Times(1);

    ((webrtc::PeerConnectionObserver*)connection_.get())->OnIceCandidate(candidate.get());
}

TEST_F(WebRtcConnectionTest, RTCStatsCollectorCallbackOnStatsDelivered) {
    rtc::scoped_refptr<webrtc::RTCStatsReport> report(webrtc::RTCStatsReport::Create(42));
    EXPECT_CALL(listener_, OnStatistic(StrEq("101"), StrEq(report->ToString())))
    .Times(1);
    
    ((webrtc::RTCStatsCollectorCallback*)connection_.get())->OnStatsDelivered(report);
}

TEST_F(WebRtcConnectionTest, CreateSessionDescriptionObserverOnSuccessWithoutInit) {
    std::unique_ptr<webrtc::SessionDescriptionInterface> descr(webrtc::CreateSessionDescription("offer", "v=0\r\no=- 2691170728423305165 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE data\r\na=msid-semantic: WMS\r\nm=application 9 DTLS/SCTP 5000\r\nc=IN IP4 0.0.0.0\r\na=ice-ufrag:+cW0\r\na=ice-pwd:Prz4hOFGOJZaxGfnAURsG9jB\r\na=fingerprint:sha-256 7B:CA:6E:FD:17:EF:C3:76:FB:F9:E7:E0:72:86:FE:1E:93:CD:F9:B1:57:4B:D6:F7:F5:35:60:E1:6C:C3:F1:A1\r\na=setup:actpass\r\na=mid:data\r\na=sctpmap:5000 webrtc-datachannel 1024\r\n", nullptr));
    
    EXPECT_CALL(listener_, OnFatalError(StrEq("101"), _))
    .Times(1);
    
    ((webrtc::CreateSessionDescriptionObserver*)connection_.get())->OnSuccess(descr.get());
}

TEST_F(WebRtcConnectionTest, CreateSessionDescriptionObserverOnSuccess) {
    Init();
    std::unique_ptr<webrtc::SessionDescriptionInterface> descr(webrtc::CreateSessionDescription("offer", "v=0\r\no=- 2691170728423305165 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\na=group:BUNDLE data\r\na=msid-semantic: WMS\r\nm=application 9 DTLS/SCTP 5000\r\nc=IN IP4 0.0.0.0\r\na=ice-ufrag:+cW0\r\na=ice-pwd:Prz4hOFGOJZaxGfnAURsG9jB\r\na=fingerprint:sha-256 7B:CA:6E:FD:17:EF:C3:76:FB:F9:E7:E0:72:86:FE:1E:93:CD:F9:B1:57:4B:D6:F7:F5:35:60:E1:6C:C3:F1:A1\r\na=setup:actpass\r\na=mid:data\r\na=sctpmap:5000 webrtc-datachannel 1024\r\n", nullptr));
    
    EXPECT_CALL(*(pc_.get()), SetLocalDescription(connection_.get(), descr.get()))
    .Times(1);
    EXPECT_CALL(client_, send_to_peer(StrEq("101"), StrEq("{\n   \"sdp\" : \"v=0\\r\\no=- 2691170728423305165 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE data\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\na=ice-ufrag:+cW0\\r\\na=ice-pwd:Prz4hOFGOJZaxGfnAURsG9jB\\r\\na=fingerprint:sha-256 7B:CA:6E:FD:17:EF:C3:76:FB:F9:E7:E0:72:86:FE:1E:93:CD:F9:B1:57:4B:D6:F7:F5:35:60:E1:6C:C3:F1:A1\\r\\na=setup:actpass\\r\\na=mid:data\\r\\na=sctpmap:5000 webrtc-datachannel 1024\\r\\n\",\n   \"type\" : \"offer\"\n}\n"), 463))
    .Times(1);
    
    ((webrtc::CreateSessionDescriptionObserver*)connection_.get())->OnSuccess(descr.get());
}


TEST_F(WebRtcConnectionTest, CreateSessionDescriptionObserverOnFailure) {
    EXPECT_CALL(listener_,
                OnFatalError(StrEq("101"),
                             StrEq("Session description failed, error: CreateSessionDescriptionObserver error msg")))
    .Times(1);
    ((webrtc::CreateSessionDescriptionObserver*)connection_.get())
    ->OnFailure("CreateSessionDescriptionObserver error msg");
}

TEST_F(WebRtcConnectionTest, SetSessionDescriptionObserverOnSuccess) {
    ((webrtc::SetSessionDescriptionObserver*)connection_.get())->OnSuccess();
}

TEST_F(WebRtcConnectionTest, SetSessionDescriptionObserverOnFailure) {
    EXPECT_CALL(listener_,
                OnFatalError(StrEq("101"),
                             StrEq("Session description failed, error: SetSessionDescriptionObserver error msg")))
    .Times(1);
    ((webrtc::SetSessionDescriptionObserver*)connection_.get())->OnFailure("SetSessionDescriptionObserver error msg");
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnOpened) {
    EXPECT_CALL(listener_, OnConnected(StrEq("101")))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnOpened();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnOpenedSecondChannel) {
    EXPECT_CALL(listener_, OnConnected(StrEq("101")))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnOpened();
    ((WebRtcChannelListener*)connection_.get())->OnOpened();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnClosed) {
    ((WebRtcChannelListener*)connection_.get())->OnOpened();
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnClosed();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnClosedTwiceOneChannel) {
    ((WebRtcChannelListener*)connection_.get())->OnOpened();
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnClosed();
    ((WebRtcChannelListener*)connection_.get())->OnClosed();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnClosedTwiceForTwoChannels) {
    ((WebRtcChannelListener*)connection_.get())->OnOpened();
    ((WebRtcChannelListener*)connection_.get())->OnOpened();
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnClosed();
    ((WebRtcChannelListener*)connection_.get())->OnClosed();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnClosedAfterOnIceConnectionChangeDisconnected) {
    EXPECT_CALL(listener_, OnDisconnected(StrEq("101")))
    .Times(1);
    ((webrtc::PeerConnectionObserver*)connection_.get())
    ->OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected);
    ((WebRtcChannelListener*)connection_.get())->OnClosed();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnMessage) {
    EXPECT_CALL(listener_, OnMessage(StrEq("101"), StrEq("data")))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnMessage("data");
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnBufferedAmountChangedWithoutChannels) {
    EXPECT_CALL(listener_, OnBufferedAmountChanged(StrEq("101"),0))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnBufferedAmountChanged();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnBufferedAmountChanged) {
    InitAndCreateOffer();
    EXPECT_CALL(listener_, OnBufferedAmountChanged(StrEq("101"),444))
    .Times(1);
    EXPECT_CALL(*(channel_.get()), GetBufferedAmount())
    .Times(4)
    .WillRepeatedly(Return(111));
    ((WebRtcChannelListener*)connection_.get())->OnBufferedAmountChanged();
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnError) {
    EXPECT_CALL(listener_, OnError(StrEq("101"), "error"))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnError("error");
}

TEST_F(WebRtcConnectionTest, WebRtcChannelListenerOnFatalError) {
    EXPECT_CALL(listener_, OnFatalError(StrEq("101"), "fatal error"))
    .Times(1);
    ((WebRtcChannelListener*)connection_.get())->OnFatalError("fatal error");
}
*/
