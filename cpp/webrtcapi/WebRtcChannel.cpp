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

#include "api/peerconnectioninterface.h"

#include "WebRtcChannel.h"
#include "WebRtcChannelListener.h"
#include "RefPtrDisposer.h"

namespace {
    class ChannelRefPtrDisposer {
    public:
        ChannelRefPtrDisposer(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) : channel_(channel) {}
        ~ChannelRefPtrDisposer() {
            if (channel_->state() != webrtc::DataChannelInterface::kClosed) {
                rtc::Thread::Current()->Dispose(new ChannelRefPtrDisposer(channel_));
            }
            channel_ = nullptr;
        }
    private:
        rtc::scoped_refptr<webrtc::DataChannelInterface> channel_;
    };
}

WebRtcChannel::WebRtcChannel() : channel_(nullptr), listener_(nullptr) {}

WebRtcChannel::WebRtcChannel(webrtc::PeerConnectionInterface* peerConnection, WebRtcChannelListener* listener) 
 : listener_(listener) {
    struct webrtc::DataChannelInit init;
    init.ordered = false;
    init.reliable = true;
    channel_ = peerConnection->CreateDataChannel("DataChannel_" + std::to_string((int64_t)this), &init);
}

WebRtcChannel::WebRtcChannel(DataChannelRefPtr channel, WebRtcChannelListener* listener)
    : channel_(channel), listener_(listener) {
}

WebRtcChannel::~WebRtcChannel() {
    RTC_LOG_F(LS_VERBOSE);
}

void WebRtcChannel::InitChannel() {
    if (channel_.get()) {
        channel_->RegisterObserver(this);
        RTC_LOG_F(LS_INFO) << "Succeeds to create data channel";
    } else {
        std::string msg = "Failed to create data channel";
        RTC_LOG_F(LS_WARNING) << msg;
    }
}

void WebRtcChannel::Close() {
    listener_ = nullptr;
    if (!channel_.get()) return;
    channel_->UnregisterObserver();
    channel_->Close();
    rtc::Thread::Current()->Dispose(new ChannelRefPtrDisposer(channel_));
    channel_ = nullptr;
}

void WebRtcChannel::Send(const rtc::CopyOnWriteBuffer& data, bool binary) {
    if (!channel_.get()) {
        RTC_LOG_F(LS_WARNING) << "Send called without opened data channel";
        return;
    }
    if (channel_->state() != webrtc::DataChannelInterface::kOpen) {
        RTC_LOG_F(LS_WARNING) << "Send called on closed data channel";
        return;
    }
    webrtc::DataBuffer buf(data, binary);
    channel_->Send(buf);
}

bool WebRtcChannel::IsOpen() {
    return channel_.get() && channel_->state() == webrtc::DataChannelInterface::kOpen;
}

uint64_t WebRtcChannel::GetBufferedAmount() {
    return channel_.get() ? channel_->buffered_amount() : 0;
}

// DataChannelObserver interface
void WebRtcChannel::OnMessage(const webrtc::DataBuffer& buffer) {
    std::string message((const char*)(buffer.data.data()), buffer.data.size());
    if(listener_) listener_->OnMessage(message);
    else RTC_LOG_F(LS_WARNING) << "Received channel message, but listener is nullptr";
}

void WebRtcChannel::OnStateChange() {
    if (channel_.get() == nullptr) return;
    webrtc::DataChannelInterface::DataState state = channel_->state();
    if (state == webrtc::DataChannelInterface::kOpen) {
        RTC_LOG_F(LS_INFO) << "WebRtcChannel::OnStateChange kOpen";
        if(listener_) listener_->OnOpened();
    } else if (state == webrtc::DataChannelInterface::kClosed || state == webrtc::DataChannelInterface::kClosing) {
        RTC_LOG_F(LS_INFO) << "WebRtcChannel::OnStateChange kClosed";
        if (listener_) listener_->OnClosed();
    }
}

void WebRtcChannel::OnBufferedAmountChange(uint64_t previous_amount) {
    if(listener_ && channel_.get() != nullptr) listener_->OnBufferedAmountChanged();
}
