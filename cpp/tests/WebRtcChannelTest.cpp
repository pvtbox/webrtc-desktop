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

#include "WebRtcChannel.h"
#include "DataChannelInterfaceMock.h"
#include "PeerConnectionMock.h"
#include "WebRtcChannelListenerMock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::NotNull;
using ::testing::InvokeWithoutArgs;

class WebRtcChannelTest: public ::testing::Test {
protected:
    void SetUp() {
        rtc::LogMessage::LogToDebug(rtc::LS_NONE);
        dc_ = new rtc::RefCountedObject<DataChannelInterfaceMock>();
        pc_ = new rtc::RefCountedObject<PeerConnectionMock>();
    }
    
    void TearDown() {
        EXPECT_CALL(*(pc_.get()), Destructor())
        .Times(1);
        EXPECT_CALL(*(dc_.get()), Destructor())
        .Times(1);
    }
    
    rtc::scoped_refptr<WebRtcChannel> channel_;
    rtc::scoped_refptr<DataChannelInterfaceMock> dc_;
    rtc::scoped_refptr<PeerConnectionMock> pc_;
    WebRtcChannelListenerMock listener_;
    
public:
    void CreateUninitChannel();
    void CreateChannelFromPC();
    void CreateChannelFromDC();
};

void WebRtcChannelTest::CreateUninitChannel() {
    channel_ = new rtc::RefCountedObject<WebRtcChannel>();
}

void WebRtcChannelTest::CreateChannelFromPC() {
    EXPECT_CALL(*(pc_.get()), CreateDataChannel(_,_))
    .Times(1)
    .WillOnce(Return(dc_));
    channel_ = new rtc::RefCountedObject<WebRtcChannel>(pc_.get(), &listener_);
}

void WebRtcChannelTest::CreateChannelFromDC() {
    channel_ = new rtc::RefCountedObject<WebRtcChannel>((rtc::scoped_refptr<webrtc::DataChannelInterface>)dc_,
                                                        &listener_);
}

MATCHER_P2(DataEq, buf, binary, "") {
    rtc::CopyOnWriteBuffer actual = arg.data;
    return actual == buf && arg.binary == binary;
}

TEST_F(WebRtcChannelTest, CreateChannelFromPC) {
    CreateChannelFromPC();
}

TEST_F(WebRtcChannelTest, CreateChannelFromDC) {
    CreateChannelFromDC();
}

TEST_F(WebRtcChannelTest, InitChannel) {
    CreateChannelFromPC();
    EXPECT_CALL(*(dc_.get()), RegisterObserver(channel_.get()))
    .Times(1);
    channel_->InitChannel();
    
    CreateChannelFromDC();
    EXPECT_CALL(*(dc_.get()), RegisterObserver(channel_.get()))
    .Times(1);
    channel_->InitChannel();
    
    CreateUninitChannel();
    channel_->InitChannel();
    
    channel_ = new rtc::RefCountedObject<WebRtcChannel>((rtc::scoped_refptr<webrtc::DataChannelInterface>)nullptr,
                                                        &listener_);
    EXPECT_CALL(listener_, OnFatalError(_))
    .Times(1);
    channel_->InitChannel();
}

TEST_F(WebRtcChannelTest, Close) {
    CreateChannelFromDC();
    
    EXPECT_CALL(*(dc_.get()), UnregisterObserver())
    .Times(1);
    EXPECT_CALL(*(dc_.get()), Close())
    .Times(1);
    channel_->Close();
    
    EXPECT_CALL(*(dc_.get()), state())
    .Times(3)
    .WillOnce(Return(webrtc::DataChannelInterface::kConnecting))
    .WillOnce(Return(webrtc::DataChannelInterface::kOpen))
    .WillOnce(Return(webrtc::DataChannelInterface::kClosed));
    rtc::Thread::Current()->ProcessMessages(1);
}

TEST_F(WebRtcChannelTest, CloseUninit) {
    CreateUninitChannel();
    channel_->Close();
}

TEST_F(WebRtcChannelTest, SendOk) {
    rtc::CopyOnWriteBuffer buf;
    buf.AppendData("data", 4);
    CreateChannelFromDC();
    EXPECT_CALL(*(dc_.get()), state())
    .Times(2)
    .WillRepeatedly(Return(webrtc::DataChannelInterface::kOpen));
    EXPECT_CALL(*(dc_.get()), Send(DataEq(buf, true)))
    .Times(1);
    channel_->Send(buf, true);
    EXPECT_CALL(*(dc_.get()), Send(DataEq(buf, false)))
    .Times(1);
    channel_->Send(buf, false);
}

TEST_F(WebRtcChannelTest, SendUninit) {
    rtc::CopyOnWriteBuffer buf;
    buf.AppendData("data", 4);
    CreateUninitChannel();
    
    channel_->Send(buf, true);
    channel_->Send(buf, false);
    
    channel_ = new rtc::RefCountedObject<WebRtcChannel>((rtc::scoped_refptr<webrtc::DataChannelInterface>)nullptr,
                                                        &listener_);
    EXPECT_CALL(listener_, OnError(_))
    .Times(2);

    channel_->Send(buf, true);
    channel_->Send(buf, false);
}

TEST_F(WebRtcChannelTest, SendNotConnected) {
    rtc::CopyOnWriteBuffer buf;
    buf.AppendData("data", 4);
    CreateChannelFromDC();
    EXPECT_CALL(*(dc_.get()), state())
    .Times(3)
    .WillOnce(Return(webrtc::DataChannelInterface::kConnecting))
    .WillOnce(Return(webrtc::DataChannelInterface::kClosing))
    .WillOnce(Return(webrtc::DataChannelInterface::kClosed));
    
    EXPECT_CALL(listener_, OnError(_))
    .Times(3);
    
    channel_->Send(buf, true);
    channel_->Send(buf, false);
    channel_->Send(buf, true);
}

TEST_F(WebRtcChannelTest, IsOpenWithoutChannel) {
    CreateUninitChannel();
    ASSERT_FALSE(channel_->IsOpen());
}

TEST_F(WebRtcChannelTest, IsOpen) {
    CreateChannelFromDC();
    EXPECT_CALL(*(dc_.get()), state())
    .Times(4)
    .WillOnce(Return(webrtc::DataChannelInterface::kOpen))
    .WillOnce(Return(webrtc::DataChannelInterface::kConnecting))
    .WillOnce(Return(webrtc::DataChannelInterface::kClosing))
    .WillOnce(Return(webrtc::DataChannelInterface::kClosed));
    ASSERT_TRUE(channel_->IsOpen());
    ASSERT_FALSE(channel_->IsOpen());
    ASSERT_FALSE(channel_->IsOpen());
    ASSERT_FALSE(channel_->IsOpen());
}

TEST_F(WebRtcChannelTest, GetBufferedAmountWithoutChannel) {
    CreateUninitChannel();
    ASSERT_EQ(0, channel_->GetBufferedAmount());
}

TEST_F(WebRtcChannelTest, GetBufferedAmount) {
    CreateChannelFromDC();
    EXPECT_CALL(*(dc_.get()), buffered_amount())
    .Times(2)
    .WillOnce(Return(100))
    .WillOnce(Return(0));
    ASSERT_EQ(100, channel_->GetBufferedAmount());
    ASSERT_EQ(0, channel_->GetBufferedAmount());
}

TEST_F(WebRtcChannelTest, DataChannelObserverOnMessage) {
    CreateChannelFromDC();
    webrtc::DataBuffer buffer("data");
    
    EXPECT_CALL(listener_, OnMessage(StrEq("data")))
    .Times(1);
    
    ((webrtc::DataChannelObserver*)channel_.get())->OnMessage(buffer);
}

TEST_F(WebRtcChannelTest, DataChannelObserverOnMessageUninit) {
    CreateUninitChannel();
    webrtc::DataBuffer buffer("data");
    ((webrtc::DataChannelObserver*)channel_.get())->OnMessage(buffer);
}

TEST_F(WebRtcChannelTest, DataChannelObserverOnStateChange) {
    CreateChannelFromDC();
    
    EXPECT_CALL(listener_, OnOpened())
    .Times(1);
    EXPECT_CALL(listener_, OnClosed())
    .Times(1);
    
    EXPECT_CALL(*(dc_.get()), state())
    .Times(4)
    .WillOnce(Return(webrtc::DataChannelInterface::kConnecting))
    .WillOnce(Return(webrtc::DataChannelInterface::kOpen))
    .WillOnce(Return(webrtc::DataChannelInterface::kClosing))
    .WillOnce(Return(webrtc::DataChannelInterface::kClosed));
    
    ((webrtc::DataChannelObserver*)channel_.get())->OnStateChange();
    ((webrtc::DataChannelObserver*)channel_.get())->OnStateChange();
    ((webrtc::DataChannelObserver*)channel_.get())->OnStateChange();
    ((webrtc::DataChannelObserver*)channel_.get())->OnStateChange();
}

TEST_F(WebRtcChannelTest, DataChannelObserverOnStateChangeUninit) {
    CreateUninitChannel();
    ((webrtc::DataChannelObserver*)channel_.get())->OnStateChange();
}

TEST_F(WebRtcChannelTest, DataChannelObserverOnBufferedAmountChange) {
    CreateChannelFromDC();
    
    EXPECT_CALL(listener_, OnBufferedAmountChanged())
    .Times(1);
    
    ((webrtc::DataChannelObserver*)channel_.get())->OnBufferedAmountChange(333);
}

TEST_F(WebRtcChannelTest, DataChannelObserverOnBufferedAmountChangeUninit) {
    CreateUninitChannel();
    ((webrtc::DataChannelObserver*)channel_.get())->OnBufferedAmountChange(333);
}
*/
