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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <rtc_base/thread.h>
#include <rtc_base/logging.h>

#include "webrtc_api.h"
#include "WebRtcListenerMock.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::StrEq;
using ::testing::WithArg;
using ::testing::InSequence;
using ::testing::DoAll;

class SandBoxTest: public ::testing::Test {
protected:
    void SetUp() {
        exit1 = false;
        exit2 = false;
        
        connection_id = "sand_box_test_connection";
        
        EXPECT_CALL(listener1, on_local_description(_,_,_))
        .WillRepeatedly(Invoke(webrtc2.get(), &WebRtc::set_remote_description));
        EXPECT_CALL(listener2, on_local_description(_,_,_))
        .WillRepeatedly(Invoke(webrtc1.get(), &WebRtc::set_remote_description));
        EXPECT_CALL(listener1, on_candidate(_,_,_,_))
        .WillRepeatedly(Invoke(webrtc2.get(), &WebRtc::set_candidate));
        EXPECT_CALL(listener2, on_candidate(_,_,_,_))
        .WillRepeatedly(Invoke(webrtc1.get(), &WebRtc::set_candidate));
        
        webrtc1.reset(new WebRtc());
        webrtc2.reset(new WebRtc());
        
        webrtc1->set_listener(&listener1);
        webrtc2->set_listener(&listener2);
       
        WebRtc::set_log_level(WebRtc::INFO);
    }
    
    void TearDown() {
        std::cerr << std::endl << "TearDown" << std::endl << std::endl;
        webrtc1->close();
        webrtc2->close();
    }
    
    std::unique_ptr<WebRtc> webrtc1;
    std::unique_ptr<WebRtc> webrtc2;
    WebRtcListenerMock listener1;
    WebRtcListenerMock listener2;
    
    std::string connection_id;
    
    volatile bool exit1;
    volatile bool exit2;
    
public:
    void Wait() {
        while(!exit1 || !exit2) {
            rtc::Thread::Current()->SleepMs(1);
        }
    }
    void Exit1() { exit1 = true; }
    void Exit2() { exit2 = true; }
    
    void Send1(const char* id) {
        webrtc1->send(id, "test data 1", 11, false);
    }
    void Send2(const char* id) {
        webrtc2->send(id, "test data 2", 11, true);
    }
    
    void Disconnect1(std::string id) {
        webrtc1->disconnect(id.c_str());
    }
    
    void Disconnect2(std::string id) {
        webrtc2->disconnect(id.c_str());
    }
    
    void MakeConnectionsPair(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
    
    void MakeConnectionPairAndSend(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send1));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send2));
        
        EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
        
        EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc2->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
    
    void MakeConnectionsPairAndDisconnect1(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Disconnect1));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
        
        EXPECT_CALL(listener2, on_disconnected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc2->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
    
    void MakeConnectionsPairAndDisconnect2(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Disconnect2));
        
        EXPECT_CALL(listener1, on_disconnected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc2->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
    
    void MakeConnectionsPairAndDisconnectBoth(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(DoAll(Invoke(this, &SandBoxTest::Disconnect1),
                        InvokeWithoutArgs(this, &SandBoxTest::Exit1)));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(DoAll(Invoke(this, &SandBoxTest::Disconnect2),
                        InvokeWithoutArgs(this, &SandBoxTest::Exit2)));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc2->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
    
    void MakeConnectionsPairSendAndDisconnect1(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send1));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send2));
        
        EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
        .Times(1)
        .WillOnce(testing::WithArg<0>(Invoke(this, &SandBoxTest::Disconnect1)));
        
        EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
        
        EXPECT_CALL(listener2, on_disconnected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc2->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
    
    void MakeConnectionsPairSendAndDisconnect2(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send1));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send2));
        
        EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
        
        EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
        .Times(1)
        .WillOnce(testing::WithArg<0>(Invoke(this, &SandBoxTest::Disconnect2)));
        
        EXPECT_CALL(listener1, on_disconnected(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc2->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
    
    void MakeConnectionsPairSendAndDisconnectBoth(std::string conn_id) {
        EXPECT_CALL(listener1, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send1));
        
        EXPECT_CALL(listener2, on_connected(_))
        .Times(1)
        .WillOnce(Invoke(this, &SandBoxTest::Send2));
        
        EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
        .Times(1)
        .WillOnce(DoAll(WithArg<0>(Invoke(this, &SandBoxTest::Disconnect1)),
                        InvokeWithoutArgs(this, &SandBoxTest::Exit1)));
        
        EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
        .Times(1)
        .WillOnce(DoAll(WithArg<0>(Invoke(this, &SandBoxTest::Disconnect2)),
                        InvokeWithoutArgs(this, &SandBoxTest::Exit2)));
        
        webrtc1->create_connection(conn_id.c_str());
        webrtc2->create_connection(conn_id.c_str());
        webrtc1->initiate_connection(conn_id.c_str());
        Wait();
    }
};

TEST_F(SandBoxTest, Connect) {
    MakeConnectionsPair(connection_id);
}

TEST_F(SandBoxTest, ConnectSend) {
    MakeConnectionPairAndSend(connection_id);
}

TEST_F(SandBoxTest, Send10kMessagesBothDirections) {
    EXPECT_CALL(listener1, on_connected(_))
    .Times(1)
    .WillOnce(Invoke(this, &SandBoxTest::Send1));
    
    EXPECT_CALL(listener2, on_connected(_))
    .Times(1)
    .WillOnce(Invoke(this, &SandBoxTest::Send2));
    
    EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    
    EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
    
    EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
    .Times(5000)
    .WillRepeatedly(WithArg<0>(Invoke(this, &SandBoxTest::Send1)))
    .RetiresOnSaturation();
    
    EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
    .Times(5000)
    .WillRepeatedly(WithArg<0>(Invoke(this, &SandBoxTest::Send2)))
    .RetiresOnSaturation();
    
    webrtc1->create_connection(connection_id.c_str());
    webrtc2->create_connection(connection_id.c_str());
    webrtc1->initiate_connection(connection_id.c_str());
    Wait();
}

TEST_F(SandBoxTest, Send10kMessagesOneDirection) {
    // webrtc1 -> webrtc2
    EXPECT_CALL(listener1, on_connected(_))
    .Times(1)
    .WillOnce(Invoke(this, &SandBoxTest::Send1));
    
    EXPECT_CALL(listener2, on_connected(_))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    
    EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
    
    EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
    .Times(5000)
    .WillRepeatedly(WithArg<0>(Invoke(this, &SandBoxTest::Send1)))
    .RetiresOnSaturation();
    
    webrtc1->create_connection(connection_id.c_str());
    webrtc2->create_connection(connection_id.c_str());
    webrtc1->initiate_connection(connection_id.c_str());
    Wait();
    
    exit1 = false;
    
    // webrtc2 -> webrtc1
    EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    
    EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
    .Times(5000)
    .WillRepeatedly(WithArg<0>(Invoke(this, &SandBoxTest::Send2)))
    .RetiresOnSaturation();
    
    Send2(connection_id.c_str());
    Wait();
}


TEST_F(SandBoxTest, 8Connections) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionsPair(connection_id.append(&i));
        std::cerr << std::endl << "Connection made" << std::endl << std::endl;
    }
}

TEST_F(SandBoxTest, 8ConnectSend) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionPairAndSend(connection_id.append(&i));
    }
}

//https://bugs.chromium.org/p/webrtc/issues/detail?id=3792
TEST_F(SandBoxTest, DISABLED_ConnectDisconnect1) {
    MakeConnectionsPairAndDisconnect1(connection_id);
}

TEST_F(SandBoxTest, ConnectDisconnect2) {
    MakeConnectionsPairAndDisconnect2(connection_id);
}

//https://bugs.chromium.org/p/webrtc/issues/detail?id=3792
TEST_F(SandBoxTest, DISABLED_ConnectDisconnectBoth) {
    MakeConnectionsPairAndDisconnectBoth(connection_id);
}

//https://bugs.chromium.org/p/webrtc/issues/detail?id=3792
TEST_F(SandBoxTest, DISABLED_8ConnectDisconnect1) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionsPairAndDisconnect1(connection_id.append(&i));
    }
    rtc::Thread::Current()->SleepMs(500);
}

TEST_F(SandBoxTest, 8ConnectDisconnect2) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionsPairAndDisconnect2(connection_id.append(&i));
    }
    rtc::Thread::Current()->SleepMs(500);
}

//https://bugs.chromium.org/p/webrtc/issues/detail?id=3792
TEST_F(SandBoxTest, DISABLED_8ConnectDisconnectBoth) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionsPairAndDisconnectBoth(connection_id.append(&i));
    }
    rtc::Thread::Current()->SleepMs(500);
}


TEST_F(SandBoxTest, ConnectSendDisconnect1) {
    MakeConnectionsPairSendAndDisconnect1(connection_id);
}

TEST_F(SandBoxTest, ConnectSendDisconnect2) {
    MakeConnectionsPairSendAndDisconnect2(connection_id);
}

TEST_F(SandBoxTest, ConnectSendDisconnectBoth) {
    MakeConnectionsPairSendAndDisconnectBoth(connection_id);
}

TEST_F(SandBoxTest, DISABLED_8ConnectSendDisconnect1) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionsPairSendAndDisconnect1(connection_id.append(&i));
    }
    rtc::Thread::Current()->SleepMs(500);
}

TEST_F(SandBoxTest, 8ConnectSendDisconnect2) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionsPairSendAndDisconnect2(connection_id.append(&i));
    }
    rtc::Thread::Current()->SleepMs(500);
}

TEST_F(SandBoxTest, 8ConnectSendDisconnectBoth) {
    for (char i = 'a'; i < 'a'+8; i++) {
        exit1 = false;
        exit2 = false;
        MakeConnectionsPairSendAndDisconnectBoth(connection_id.append(&i));
    }
    rtc::Thread::Current()->SleepMs(500);
}
/*
TEST_F(SandBoxTest, SendWithoutConnect) {
    webrtc1->send("111", "data", 4, true);
    webrtc2->send("222", "data", 4, false);
    Wait();
}

TEST_F(SandBoxTest, SendWrongConnection) {
    MakeConnectionsPair(connection_id);
    
    exit1 = false;
    exit2 = false;
    
    EXPECT_CALL(listener1, on_error(StrEq("111"), WebRtcListener::Error, _, _))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    webrtc1->send("111", "data", 4, true);
    EXPECT_CALL(listener2, on_error(StrEq("222"), WebRtcListener::Error, _, _))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
    webrtc2->send("222", "data", 4, false);
    Wait();
}

TEST_F(SandBoxTest, DisconnectWithoutConnect) {
    EXPECT_CALL(listener1, on_error(StrEq("111"), WebRtcListener::Error, _, _))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    webrtc1->disconnect("111");
    EXPECT_CALL(listener2, on_error(StrEq("222"), WebRtcListener::Error, _, _))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
    webrtc2->disconnect("222");
    Wait();
}

TEST_F(SandBoxTest, DisconnectWrongConnection) {
    MakeConnectionsPair(connection_id);
    
    exit1 = false;
    exit2 = false;
    
    EXPECT_CALL(listener1, on_error(StrEq("111"), WebRtcListener::Error, _, _))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    webrtc1->disconnect("111");
    EXPECT_CALL(listener2, on_error(StrEq("222"), WebRtcListener::Error, _, _))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
    webrtc2->disconnect("222");
    Wait();
}

TEST_F(SandBoxTest, SendWorksAfterDisconnectWrongConnection) {
    MakeConnectionPairAndSend(connection_id);
    
    exit1 = false;
    exit2 = false;
    
    EXPECT_CALL(listener1, on_error(StrEq("111"), WebRtcListener::Error, _, _))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    webrtc1->disconnect("111");
    
    EXPECT_CALL(listener2, on_error(StrEq("222"), WebRtcListener::Error, _, _))
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
    webrtc2->disconnect("222");
    
    Wait();
    
    exit1 = false;
    exit2 = false;
    EXPECT_CALL(listener1, on_message(_, StrEq("test data 2"), 11))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit1));
    
    EXPECT_CALL(listener2, on_message(_, StrEq("test data 1"), 11))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &SandBoxTest::Exit2));
    
    webrtc1->send(connection_id.c_str(), "test data 1", 11, true);
    webrtc2->send(connection_id.c_str(), "test data 2", 11, false);
    Wait();
}*/
