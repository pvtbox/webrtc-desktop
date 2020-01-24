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

#include "webrtc_api.h"
#include "WebRtcProxyFactoryMock.h"
#include "WebRtcProxyMock.h"
#include "SignalServerClientMock.h"
#include "WebRtcListenerMock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::StrEq;

class WebRtcTest: public ::testing::Test {
protected:
    void SetUp() {
        proxy_ = new NiceMock<WebRtcProxyMock>();
        factory_ = new NiceMock<WebRtcProxyFactoryMock>();
        EXPECT_CALL(*factory_, Create(&client_))
        .Times(1)
        .WillOnce(Return(proxy_));
        EXPECT_CALL(*factory_, Destructor())
        .Times(1);
        webrtc_.reset(new WebRtc(&client_, factory_));
    }
    
    void TearDown() {
        if (proxy_) {
            EXPECT_CALL(*proxy_, Destructor())
            .Times(1);
        }
    }
    
    NiceMock<SignalServerClientMock> client_;
    NiceMock<WebRtcListenerMock> listener_;
    WebRtcProxyFactoryMock* factory_;
    WebRtcProxyMock* proxy_;
    std::unique_ptr<WebRtc> webrtc_;
};

TEST_F(WebRtcTest, SetListenerGoesToProxy) {
    EXPECT_CALL(*proxy_, set_listener((WebRtcListener*)nullptr))
    .Times(1);
    webrtc_->set_listener(nullptr);
    EXPECT_CALL(*proxy_, set_listener(&listener_))
    .Times(1);
    webrtc_->set_listener(&listener_);
}

TEST_F(WebRtcTest, AddIceServerGoesToProxy) {
    EXPECT_CALL(*proxy_, add_ice_server("111", "222", "333"))
    .Times(1);
    webrtc_->add_ice_server("111", "222", "333");
}

TEST_F(WebRtcTest, ConnectGoesToProxy) {
    EXPECT_CALL(*proxy_, connect(StrEq("444")))
    .Times(1);
    webrtc_->connect("444");
}

TEST_F(WebRtcTest, DisconnectGoesToProxy) {
    EXPECT_CALL(*proxy_, disconnect(StrEq("555")))
    .Times(1);
    webrtc_->disconnect("555");
}

TEST_F(WebRtcTest, SendGoesToProxy) {
    EXPECT_CALL(*proxy_, send(StrEq("666"), "data", 777, true))
    .Times(1);
    webrtc_->send("666", "data", 777, true);
    
    EXPECT_CALL(*proxy_, send(StrEq("888"), "", 999, false))
    .Times(1);
    webrtc_->send("888", "", 999, false);
}

TEST_F(WebRtcTest, RequestStatisticGoesToProxy) {
    EXPECT_CALL(*proxy_, request_statistic(StrEq("101010")))
    .Times(1);
    webrtc_->request_statistic("101010");
}

TEST_F(WebRtcTest, CloseGoesToProxyDtor) {
    EXPECT_CALL(*proxy_, Destructor())
    .Times(1);
    webrtc_->close();
    proxy_ = nullptr;
}
*/
