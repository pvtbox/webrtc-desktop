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

#include <webrtc/base/thread.h>

#include "WebRtcProxy.h"
#include "webrtc_api.h"
#include "WebRtcProxyFactoryMock.h"
#include "SignalServerClientMock.h"
#include "SignalServerClientListenerMock.h"
#include "WebRtcConnectionPoolFactoryMock.h"
#include "WebRtcConnectionPoolMock.h"
#include "WebRtcConnectionPoolListenerMock.h"
#include "WebRtcListenerMock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::NotNull;
using ::testing::InvokeWithoutArgs;

class WebRtcProxyTest: public ::testing::Test {
protected:
    void SetUp() {
        rtc::scoped_refptr<WebRtcConnectionPoolMock> pool =
            new rtc::RefCountedObject<WebRtcConnectionPoolMock>(&client_, &poolListener_);
        factory_ = new WebRtcConnectionPoolFactoryMock();
        EXPECT_CALL(*factory_, Create(_, _))
        .Times(1)
        .WillOnce(Return(pool));
        
        EXPECT_CALL(*(pool.get()), Init())
        .Times(1);
        
        EXPECT_CALL(client_, set_listener(NotNull()))
        .Times(1);
        
        proxy_.reset(new WebRtcProxy(&client_, factory_));
        
        pool_ = pool.get();
    }
    
    void TearDown() {
        rtc::Thread::Current()->SleepMs(1000);
        EXPECT_CALL(client_, set_listener(nullptr))
        .Times(1);
        EXPECT_CALL(*pool_, Close())
        .Times(1);
        EXPECT_CALL(*factory_, Destructor())
        .Times(1);
        EXPECT_CALL(*pool_, Destructor());
        proxy_.reset();
    }
    SignalServerClientMock client_;
    SignalServerClientListenerMock clientListener_;
    WebRtcConnectionPoolListenerMock poolListener_;
    WebRtcConnectionPoolMock* pool_;
    WebRtcConnectionPoolFactoryMock* factory_;
    std::unique_ptr<WebRtcProxy> proxy_;
    WebRtcListenerMock listener_;
    
public:
    void SetWebRtcListenerNull() {
        proxy_->set_listener((WebRtcListener*)nullptr);
    }
};

MATCHER_P(IceServerEq, server, "") {
    if (server.uri == arg.uri &&
        server.username == arg.username &&
        server.password == arg.password &&
        server.urls == arg.urls) {
        return true;
    }
    return false;
}

TEST_F(WebRtcProxyTest, GetSignalServerClient) {
    ASSERT_EQ(&client_, proxy_->GetSignalServerClient());
}

TEST_F(WebRtcProxyTest, AddIceServerGoesToConnectionPool) {
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "111";
    server.username = "222";
    server.password = "333";
    EXPECT_CALL(*pool_, AddIceServer(IceServerEq(server)))
    .Times(1);
    proxy_->add_ice_server(server.uri, server.username, server.password);
}

TEST_F(WebRtcProxyTest, ConnectGoesToConnectionPool) {
    EXPECT_CALL(*pool_, Connect(StrEq("111")))
    .Times(1);
    proxy_->connect("111");
}

TEST_F(WebRtcProxyTest, DisconnectGoesToConnectionPool) {
    EXPECT_CALL(*pool_, Disconnect(StrEq("111")))
    .Times(1);
    proxy_->disconnect("111");
}

TEST_F(WebRtcProxyTest, SendGoesToConnectionPool) {
    rtc::CopyOnWriteBuffer buf("data", 4);
    
    EXPECT_CALL(*pool_, Send(StrEq("111"), buf, true))
    .Times(1);
    proxy_->send("111", (const char*)buf.data(), buf.size(), true);
    EXPECT_CALL(*pool_, Send(StrEq("222"), buf, false))
    .Times(1);
    proxy_->send("222", (const char*)buf.data(), buf.size(), false);
}

TEST_F(WebRtcProxyTest, RequestStatisticGoesToConnectionPool) {
    EXPECT_CALL(*pool_, RequestStatistic(StrEq("111")))
    .Times(1);
    proxy_->request_statistic("111");
}

TEST_F(WebRtcProxyTest, SignalServerClientInterfaceProxy) {
    EXPECT_CALL(client_, send_to_peer(StrEq("111"), StrEq("222"), 333))
    .Times(1);
    ((SignalServerClient*)proxy_.get())->send_to_peer("111", "222", 333);
}

TEST_F(WebRtcProxyTest, SignalServerClientListenerInterfaceProxy) {
    ((SignalServerClient*)proxy_.get())->set_listener((SignalServerClientListener*)&clientListener_);
    EXPECT_CALL(clientListener_, on_message_from_peer(StrEq("444"), StrEq("555")))
    .Times(1);
    ((SignalServerClientListener*)proxy_.get())->on_message_from_peer("444", "555");
}

TEST_F(WebRtcProxyTest, ConnectionPoolListenerInterfaceOnConnected) {
    proxy_->set_listener((WebRtcListener*)&listener_);
    EXPECT_CALL(listener_, on_connected(StrEq("123")))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &WebRtcProxyTest::SetWebRtcListenerNull));
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnConnected("123");
    rtc::Thread::Current()->SleepMs(100);
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnConnected("123");
}

TEST_F(WebRtcProxyTest, ConnectionPoolListenerInterfaceOnDisconnected) {
    EXPECT_CALL(*pool_, Disconnect(StrEq("123")))
    .Times(2);
    proxy_->set_listener((WebRtcListener*)&listener_);
    EXPECT_CALL(listener_, on_disconnected(StrEq("123")))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &WebRtcProxyTest::SetWebRtcListenerNull));
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnDisconnected("123");
    rtc::Thread::Current()->SleepMs(100);
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnDisconnected("123");
}

TEST_F(WebRtcProxyTest, ConnectionPoolListenerInterfaceOnMessage) {
    proxy_->set_listener((WebRtcListener*)&listener_);
    EXPECT_CALL(listener_, on_message(StrEq("123"), StrEq("data"), 4))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &WebRtcProxyTest::SetWebRtcListenerNull));
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnMessage("123", std::string("data"));
    rtc::Thread::Current()->SleepMs(100);
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnMessage("123", std::string("data"));
}

TEST_F(WebRtcProxyTest, ConnectionPoolListenerInterfaceOnBufferedAmountChanged) {
    proxy_->set_listener((WebRtcListener*)&listener_);
    EXPECT_CALL(listener_, on_buffered_amount_change(StrEq("123"), 100500))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &WebRtcProxyTest::SetWebRtcListenerNull));
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnBufferedAmountChanged("123", 100500);
    rtc::Thread::Current()->SleepMs(100);
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnBufferedAmountChanged("123", 100500);
}

TEST_F(WebRtcProxyTest, ConnectionPoolListenerInterfaceOnError) {
    proxy_->set_listener((WebRtcListener*)&listener_);
    EXPECT_CALL(listener_, on_error(StrEq("123"), WebRtcListener::Error, StrEq("err"), 3))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &WebRtcProxyTest::SetWebRtcListenerNull));
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnError("123", "err");
    rtc::Thread::Current()->SleepMs(100);
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnError("123", "err");
}

TEST_F(WebRtcProxyTest, ConnectionPoolListenerInterfaceOnFatalError) {
    EXPECT_CALL(*pool_, Disconnect(StrEq("123")))
    .Times(2);
    proxy_->set_listener((WebRtcListener*)&listener_);
    EXPECT_CALL(listener_, on_error(StrEq("123"), WebRtcListener::FatalError, StrEq("err"), 3))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &WebRtcProxyTest::SetWebRtcListenerNull));
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnFatalError("123", "err");
    rtc::Thread::Current()->SleepMs(100);
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnFatalError("123", "err");
}

TEST_F(WebRtcProxyTest, ConnectionPoolListenerInterfaceOnStatistic) {
    proxy_->set_listener((WebRtcListener*)&listener_);
    EXPECT_CALL(listener_, on_statistic(StrEq("123"), StrEq("stat"), 4))
    .Times(1)
    .WillOnce(InvokeWithoutArgs(this, &WebRtcProxyTest::SetWebRtcListenerNull));
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnStatistic("123", "stat");
    rtc::Thread::Current()->SleepMs(100);
    ((WebRtcConnectionPoolListener*)proxy_.get())->OnStatistic("123", "stat");
}
 */
