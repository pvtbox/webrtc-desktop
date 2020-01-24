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

#include "WebRtcConnectionPool.h"
#include "SignalServerClientMock.h"
#include "WebRtcConnectionPoolListenerMock.h"
#include "WebRtcConnectionFactoryMock.h"
#include "WebRtcConnectionMock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::NotNull;
using ::testing::InvokeWithoutArgs;

class WebRtcConnectionPoolTest: public ::testing::Test {
protected:
    void SetUp() {
        rtc::LogMessage::LogToDebug(rtc::LS_NONE);
        factory_ = new WebRtcConnectionFactoryMock();
        pool_ = new rtc::RefCountedObject<WebRtcConnectionPool>(&client_, &listener_, factory_);
        connection_ = new WebRtcConnectionMock();
        
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
        
        data_ = rtc::CopyOnWriteBuffer("test", 4);
        
        EXPECT_CALL(*factory_, Destructor())
        .Times(1);
    }
    
    void TearDown() {
        EXPECT_CALL(*(connection_.get()), Destructor())
        .Times(1);
        connection_ = nullptr;
        pool_ = nullptr;
        rtc::Thread::Current()->ProcessMessages(1);
    }
    
    void MakeConnection(std::string);
    
    SignalServerClientMock client_;
    WebRtcConnectionPoolListenerMock listener_;
    WebRtcConnectionFactoryMock* factory_;
    rtc::scoped_refptr<WebRtcConnectionPool> pool_;
    std::vector<webrtc::PeerConnectionInterface::IceServer> ice_;
    rtc::scoped_refptr<WebRtcConnectionMock> connection_;
    rtc::CopyOnWriteBuffer data_;
};

MATCHER_P(IsIceEq, ice, "") {
    if (ice.size() != arg.size()) return false;
    
    for(int i = 0; i < ice.size(); i++) {
        webrtc::PeerConnectionInterface::IceServer expected = ice[i];
        webrtc::PeerConnectionInterface::IceServer actual = arg[i];
        if (expected.uri != actual.uri ||
            expected.username != actual.username ||
            expected.password != actual.password ||
            expected.urls != actual.urls) {
            return false;
        }
    }
    return true;
}

void WebRtcConnectionPoolTest::MakeConnection(std::string id) {
    EXPECT_CALL(*factory_, Create(id,
                                  &client_,
                                  IsIceEq(std::vector<webrtc::PeerConnectionInterface::IceServer>()),
                                  pool_.get()))
    .Times(1)
    .WillOnce(Return(connection_));
    EXPECT_CALL(*(connection_.get()), GetId())
    .Times(1)
    .WillOnce(Return(id));
    EXPECT_CALL(*(connection_.get()), CreateOffer())
    .Times(1);
    pool_->Connect(id);
}

TEST_F(WebRtcConnectionPoolTest, InitSetsClientListener) {
    EXPECT_CALL(client_, set_listener(pool_.get()))
    .Times(1);
    pool_->Init();
}

TEST_F(WebRtcConnectionPoolTest, Connect) {
    MakeConnection("111");
}

TEST_F(WebRtcConnectionPoolTest, ConnectWithIce) {
    ASSERT_GT(ice_.size(), 0);
    for (std::vector<webrtc::PeerConnectionInterface::IceServer>::const_iterator it = ice_.cbegin();
         it != ice_.cend();
         ++it) {
        pool_->AddIceServer(*it);
    }
    
    EXPECT_CALL(*factory_, Create(StrEq("111"), &client_, IsIceEq(ice_), pool_.get()))
    .Times(1)
    .WillOnce(Return(connection_));
    EXPECT_CALL(*(connection_.get()), GetId())
    .Times(1)
    .WillOnce(Return("111"));
    EXPECT_CALL(*(connection_.get()), CreateOffer())
    .Times(1);
    pool_->Connect("111");
}

TEST_F(WebRtcConnectionPoolTest, DisconnectAfterConnect) {
    MakeConnection("111");
    EXPECT_CALL(*(connection_.get()), Close())
    .Times(1);
    pool_->Disconnect("111");
}

TEST_F(WebRtcConnectionPoolTest, DisconnectWithoutConnect) {
    EXPECT_CALL(listener_, OnError(StrEq("111"), _))
    .Times(1);
    
    pool_->Disconnect("111");
}

TEST_F(WebRtcConnectionPoolTest, DisconnectWrongConnectionAfterConnect) {
    MakeConnection("111");
    
    EXPECT_CALL(listener_, OnError(StrEq("222"), _))
    .Times(1);
    
    pool_->Disconnect("222");
}

TEST_F(WebRtcConnectionPoolTest, ConnectAfterDisconnectAfterConnect) {
    MakeConnection("111");
    
    EXPECT_CALL(*(connection_.get()), Close())
    .Times(1);
    pool_->Disconnect("111");
    
    MakeConnection("111");
}

TEST_F(WebRtcConnectionPoolTest, Send) {
    MakeConnection("111");
    MakeConnection("222");
    
    EXPECT_CALL(*(connection_.get()), Send(data_, true))
    .Times(1);
    pool_->Send("111", data_, true);
    EXPECT_CALL(*(connection_.get()), Send(data_, false))
    .Times(1);
    pool_->Send("222", data_, false);
    EXPECT_CALL(*(connection_.get()), Send(data_, false))
    .Times(1);
    pool_->Send("111", data_, false);
    EXPECT_CALL(*(connection_.get()), Send(data_, true))
    .Times(1);
    pool_->Send("222", data_, true);
}

TEST_F(WebRtcConnectionPoolTest, SendWithoutConnect) {
    EXPECT_CALL(listener_, OnError(StrEq("111"), _))
    .Times(1);
    pool_->Send("111", data_, true);
    EXPECT_CALL(listener_, OnError(StrEq("222"), _))
    .Times(1);
    pool_->Send("222", data_, false);
}

TEST_F(WebRtcConnectionPoolTest, SendWrongConnection) {
    MakeConnection("111");
    EXPECT_CALL(listener_, OnError(StrEq("222"), _))
    .Times(1);
    pool_->Send("222", data_, true);
    EXPECT_CALL(listener_, OnError(StrEq("222"), _))
    .Times(1);
    pool_->Send("222", data_, false);
}

TEST_F(WebRtcConnectionPoolTest, SendAfterDisconnectAfterConnect) {
    MakeConnection("111");
    
    EXPECT_CALL(*(connection_.get()), Close())
    .Times(1);
    pool_->Disconnect("111");
    
    EXPECT_CALL(listener_, OnError(StrEq("111"), _))
    .Times(1);
    pool_->Send("111", data_, true);
    EXPECT_CALL(listener_, OnError(StrEq("111"), _))
    .Times(1);
    pool_->Send("111", data_, false);
}

TEST_F(WebRtcConnectionPoolTest, SendAfterConnectAfterDisconnectAfterConnect) {
    MakeConnection("111");
    
    EXPECT_CALL(*(connection_.get()), Close())
    .Times(1);
    pool_->Disconnect("111");
    
    MakeConnection("111");
    
    EXPECT_CALL(*(connection_.get()), Send(data_, true))
    .Times(1);
    pool_->Send("111", data_, true);
    EXPECT_CALL(*(connection_.get()), Send(data_, false))
    .Times(1);
    pool_->Send("111", data_, false);
}

TEST_F(WebRtcConnectionPoolTest, RequestStatisticWithoutConnection) {
    EXPECT_CALL(listener_, OnError(StrEq("111"), _))
    .Times(1);
    pool_->RequestStatistic("111");
}

TEST_F(WebRtcConnectionPoolTest, RequestStatistic) {
    MakeConnection("111");
    EXPECT_CALL(*(connection_.get()), RequestStatistic())
    .Times(1);
    pool_->RequestStatistic("111");
}

TEST_F(WebRtcConnectionPoolTest, RequestStatisticWrongConnection) {
    MakeConnection("111");
    EXPECT_CALL(listener_, OnError(StrEq("222"), _))
    .Times(1);
    pool_->RequestStatistic("222");
}

TEST_F(WebRtcConnectionPoolTest, Close) {
    pool_->Close();
}

TEST_F(WebRtcConnectionPoolTest, CloseWith2Connections) {
    MakeConnection("111");
    MakeConnection("222");
    EXPECT_CALL(*(connection_.get()), Close())
    .Times(2);
    pool_->Close();
}

TEST_F(WebRtcConnectionPoolTest, WebRtcConnectionListenerOnConnected) {
    EXPECT_CALL(listener_, OnConnected(StrEq("111")))
    .Times(1);
    ((WebRtcConnectionListener*)pool_.get())->OnConnected("111");
    pool_->Close();
    ((WebRtcConnectionListener*)pool_.get())->OnConnected("111");
}

TEST_F(WebRtcConnectionPoolTest, WebRtcConnectionListenerOnDisconnected) {
    EXPECT_CALL(listener_, OnDisconnected(StrEq("111")))
    .Times(1);
    ((WebRtcConnectionListener*)pool_.get())->OnDisconnected("111");
    pool_->Close();
    ((WebRtcConnectionListener*)pool_.get())->OnDisconnected("111");
}

TEST_F(WebRtcConnectionPoolTest, WebRtcConnectionListenerOnMessage) {
    EXPECT_CALL(listener_, OnMessage(StrEq("111"), StrEq("msg")))
    .Times(1);
    ((WebRtcConnectionListener*)pool_.get())->OnMessage("111", "msg");
    pool_->Close();
    ((WebRtcConnectionListener*)pool_.get())->OnMessage("111", "msg");
}

TEST_F(WebRtcConnectionPoolTest, WebRtcConnectionListenerOnBufferedAmountChanged) {
    EXPECT_CALL(listener_, OnBufferedAmountChanged(StrEq("111"), 222))
    .Times(1);
    ((WebRtcConnectionListener*)pool_.get())->OnBufferedAmountChanged("111", 222);
    pool_->Close();
    ((WebRtcConnectionListener*)pool_.get())->OnBufferedAmountChanged("111", 222);
}

TEST_F(WebRtcConnectionPoolTest, WebRtcConnectionListenerOnError) {
    EXPECT_CALL(listener_, OnError(StrEq("111"), StrEq("err")))
    .Times(1);
    ((WebRtcConnectionListener*)pool_.get())->OnError("111", "err");
    pool_->Close();
    ((WebRtcConnectionListener*)pool_.get())->OnError("111", "err");
}

TEST_F(WebRtcConnectionPoolTest, WebRtcConnectionListenerOnFatalError) {
    EXPECT_CALL(listener_, OnFatalError(StrEq("111"), StrEq("err")))
    .Times(1);
    ((WebRtcConnectionListener*)pool_.get())->OnFatalError("111", "err");
    pool_->Close();
    ((WebRtcConnectionListener*)pool_.get())->OnFatalError("111", "err");
}

TEST_F(WebRtcConnectionPoolTest, WebRtcConnectionListenerOnStatistic) {
    EXPECT_CALL(listener_, OnStatistic(StrEq("111"), StrEq("stat")))
    .Times(1);
    ((WebRtcConnectionListener*)pool_.get())->OnStatistic("111", "stat");
    pool_->Close();
    ((WebRtcConnectionListener*)pool_.get())->OnStatistic("111", "stat");
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnNullMsg) {
    EXPECT_CALL(listener_, OnError(StrEq("111"), StrEq("Received null message.")))
    .Times(1);
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", nullptr);
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnEmptyMsg) {
    std::string msg;
    EXPECT_CALL(listener_, OnError(StrEq("111"), StrEq("Received empty message.")))
    .Times(1);
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnRandomMsg) {
    EXPECT_CALL(listener_, OnError(StrEq("111"), StrEq("Received unknown message.")))
    .Times(1);
    std::string msg = "atjkygdhbulaskidjmas;i/dlk";
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnInvalidOfferMsg) {
    std::string msg = "{\n   \"sdp\" : \"\\r\\no=- 1014979726174326931 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE data\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\na=ice-ufrag:o+wZ\\r\\na=ice-pwd:Z88ZAQWAPpoUM78LsK2Wdrs8\\r\\na=fingerprint:sha-256 CD:97:F0:18:CC:05:7F:A4:20:A9:22:8E:57:BD:0F:2E:E8:C8:0B:3D:47:E2:E7:D9:11:3B:67:0F:6F:AE:71:2E\\r\\na=setup:actpass\\r\\na=mid:data\\r\\na=sctpmap:\\r\\n\",\n   \"type\" : \"offer\"\n}\n";
    
    EXPECT_CALL(listener_,
                OnError(StrEq("111"),
                        StrEq("Can't parse received session description message. SdpParseError was : Expect line: v=")))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnOfferMsg) {
    std::string msg = "{\n   \"sdp\" : \"v=0\\r\\no=- 1014979726174326931 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE data\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\na=ice-ufrag:o+wZ\\r\\na=ice-pwd:Z88ZAQWAPpoUM78LsK2Wdrs8\\r\\na=fingerprint:sha-256 CD:97:F0:18:CC:05:7F:A4:20:A9:22:8E:57:BD:0F:2E:E8:C8:0B:3D:47:E2:E7:D9:11:3B:67:0F:6F:AE:71:2E\\r\\na=setup:actpass\\r\\na=mid:data\\r\\na=sctpmap:5000 webrtc-datachannel 1024\\r\\n\",\n   \"type\" : \"offer\"\n}\n";
    
    EXPECT_CALL(*factory_, Create(StrEq("111"),
                                  &client_,
                                  IsIceEq(std::vector<webrtc::PeerConnectionInterface::IceServer>()),
                                  pool_.get()))
    .Times(1)
    .WillOnce(Return(connection_));
    EXPECT_CALL(*(connection_.get()), GetId())
    .Times(1)
    .WillRepeatedly(Return("111"));
    EXPECT_CALL(*(connection_.get()), CreateAnswer())
    .Times(1);
    EXPECT_CALL(*(connection_.get()), SetRemoteDescription(_))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnOfferMsgForExistingConnection) {
    MakeConnection("111");
    std::string msg = "{\n   \"sdp\" : \"v=0\\r\\no=- 1014979726174326931 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE data\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\na=ice-ufrag:o+wZ\\r\\na=ice-pwd:Z88ZAQWAPpoUM78LsK2Wdrs8\\r\\na=fingerprint:sha-256 CD:97:F0:18:CC:05:7F:A4:20:A9:22:8E:57:BD:0F:2E:E8:C8:0B:3D:47:E2:E7:D9:11:3B:67:0F:6F:AE:71:2E\\r\\na=setup:actpass\\r\\na=mid:data\\r\\na=sctpmap:5000 webrtc-datachannel 1024\\r\\n\",\n   \"type\" : \"offer\"\n}\n";
    
    EXPECT_CALL(*(connection_.get()), CreateAnswer())
    .Times(1);
    EXPECT_CALL(*(connection_.get()), SetRemoteDescription(_))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnInvalidAnswerMsg) {
    std::string msg = "{\"sdp\" : \"\\r\\no=- 6746309946199608191 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\nb=AS:30\\r\\na=ice-ufrag:4cY7\\r\\na=ice-pwd:S5LiXEba42llCs6WxP3aqd3s\\r\\na=fingerprint:sha-256 99:C6:12:BA:19:49:34:45:F6:AB:02:4F:B0:25:1C:44:EF:74:8D:D8:24:EA:86:2F:34:32:38:E1:AE:F6:CF:85\\r\\na=setup:active\\r\\na=mid:data\\r\\na=sctpmap:5000 webrtc-datachannel 1024\\r\\n\",\"type\" : \"answer\"}";
    
    EXPECT_CALL(listener_, OnError(StrEq("111"), _))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnAnswerMsgWithoutConnection) {
    std::string msg = "{\"sdp\" : \"v=0\\r\\no=- 6746309946199608191 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\nb=AS:30\\r\\na=ice-ufrag:4cY7\\r\\na=ice-pwd:S5LiXEba42llCs6WxP3aqd3s\\r\\na=fingerprint:sha-256 99:C6:12:BA:19:49:34:45:F6:AB:02:4F:B0:25:1C:44:EF:74:8D:D8:24:EA:86:2F:34:32:38:E1:AE:F6:CF:85\\r\\na=setup:active\\r\\na=mid:data\\r\\na=sctpmap:5000 webrtc-datachannel 1024\\r\\n\",\"type\" : \"answer\"}";
    
    EXPECT_CALL(*factory_, Create(_,_,_,_))
    .Times(0);
    EXPECT_CALL(listener_, OnError(StrEq("111"),_))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnAnswerMsgWithConnection) {
    MakeConnection("111");
    std::string msg = "{\"sdp\" : \"v=0\\r\\no=- 6746309946199608191 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\nb=AS:30\\r\\na=ice-ufrag:4cY7\\r\\na=ice-pwd:S5LiXEba42llCs6WxP3aqd3s\\r\\na=fingerprint:sha-256 99:C6:12:BA:19:49:34:45:F6:AB:02:4F:B0:25:1C:44:EF:74:8D:D8:24:EA:86:2F:34:32:38:E1:AE:F6:CF:85\\r\\na=setup:active\\r\\na=mid:data\\r\\na=sctpmap:5000 webrtc-datachannel 1024\\r\\n\",\"type\" : \"answer\"}";
    
    EXPECT_CALL(*(connection_.get()), SetRemoteDescription(_))
    .Times(1);
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnAnswerMsgWithWrongConnection) {
    MakeConnection("111");
    std::string msg = "{\"sdp\" : \"v=0\\r\\no=- 6746309946199608191 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=msid-semantic: WMS\\r\\nm=application 9 DTLS/SCTP 5000\\r\\nc=IN IP4 0.0.0.0\\r\\nb=AS:30\\r\\na=ice-ufrag:4cY7\\r\\na=ice-pwd:S5LiXEba42llCs6WxP3aqd3s\\r\\na=fingerprint:sha-256 99:C6:12:BA:19:49:34:45:F6:AB:02:4F:B0:25:1C:44:EF:74:8D:D8:24:EA:86:2F:34:32:38:E1:AE:F6:CF:85\\r\\na=setup:active\\r\\na=mid:data\\r\\na=sctpmap:5000 webrtc-datachannel 1024\\r\\n\",\"type\" : \"answer\"}";
    
    EXPECT_CALL(*factory_, Create(_,_,_,_))
    .Times(0);
    EXPECT_CALL(listener_, OnError(StrEq("222"),_))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("222", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnCandidateMsgWithoutConnection) {
    std::string msg = "{\"candidate\" : \"candidate:777009151 1 udp 2122260223 192.168.20.5 55807 typ host generation 0 ufrag 4cY7 network-id 1 network-cost 50\", \"sdpMLineIndex\" : 0, \"sdpMid\" : \"data\"}";
    
    EXPECT_CALL(*factory_, Create(_,_,_,_))
    .Times(0);
    EXPECT_CALL(listener_, OnError(StrEq("111"),_))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnInvalidCandidateMsg) {
    MakeConnection("111");
    std::string msg = "{\"candidate\" : \"candidate:7770091512122260223 192.168.20.5 55807 typ host generation 0 ufrag 4cY7 network-id 1 network-cost 50\", \"sdpMLineIndex\" : 0, \"sdpMid\" : \"data\"}";
    
    EXPECT_CALL(*(connection_.get()), GetId())
    .Times(1)
    .WillRepeatedly(Return("111"));
    
    EXPECT_CALL(listener_, OnError(StrEq("111"), _))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnCandidateMsgWithConnectionIceOk) {
    MakeConnection("111");
    std::string msg = "{\"candidate\" : \"candidate:777009151 1 udp 2122260223 192.168.20.5 55807 typ host generation 0 ufrag 4cY7 network-id 1 network-cost 50\", \"sdpMLineIndex\" : 0, \"sdpMid\" : \"data\"}";
    EXPECT_CALL(*factory_, Create(_,_,_,_))
    .Times(0);
    EXPECT_CALL(*(connection_.get()), AddIceCandidate(_))
    .Times(1)
    .WillOnce(Return(true));
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnCandidateMsgWithConnectionIceFailed) {
    MakeConnection("111");
    std::string msg = "{\"candidate\" : \"candidate:777009151 1 udp 2122260223 192.168.20.5 55807 typ host generation 0 ufrag 4cY7 network-id 1 network-cost 50\", \"sdpMLineIndex\" : 0, \"sdpMid\" : \"data\"}";
    EXPECT_CALL(*factory_, Create(_,_,_,_))
    .Times(0);
    EXPECT_CALL(*(connection_.get()), AddIceCandidate(_))
    .Times(1)
    .WillOnce(Return(false));
    EXPECT_CALL(*(connection_.get()), GetId())
    .Times(1)
    .WillRepeatedly(Return("111"));
    EXPECT_CALL(listener_, OnError(StrEq("111"),_))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}

TEST_F(WebRtcConnectionPoolTest, SignalServerClientListenerOnCandidateMsgWithWrongConnection) {
    MakeConnection("222");
    std::string msg = "{\"candidate\" : \"candidate:777009151 1 udp 2122260223 192.168.20.5 55807 typ host generation 0 ufrag 4cY7 network-id 1 network-cost 50\", \"sdpMLineIndex\" : 0, \"sdpMid\" : \"data\"}";
    
    EXPECT_CALL(*factory_, Create(_,_,_,_))
    .Times(0);
    EXPECT_CALL(listener_, OnError(StrEq("111"),_))
    .Times(1);
    
    ((SignalServerClientListener*)pool_.get())->on_message_from_peer("111", msg.c_str());
}
*/
