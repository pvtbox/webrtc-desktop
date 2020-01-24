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

#include <cassert>

#include "webrtc_api.h"
#include "WebRtcConnection.h"
#include "WebRtcConnectionListener.h"
#include "WebRtcChannelFactory.h"
#include "WebRtcChannel.h"
#include "RefPtrDisposer.h"


WebRtcConnection::WebRtcConnection(std::string id, 
                                   WebRtcConnectionListener* listener,
                                   WebRtcChannelFactory* channelFactory)
 : id_(id),
   listener_(listener),
   channelSelector_(0),
   channelsOpened_(0),
   channelFactory_(channelFactory) {
     if (channelFactory_.get() == nullptr) {
         channelFactory_.reset(new WebRtcChannelFactory());
     }
 }

void WebRtcConnection::Init(webrtc::PeerConnectionFactoryInterface* factory, IceServers ice) {
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.servers = ice;

    peerConnection_ = factory->CreatePeerConnection(config, nullptr, nullptr, this);
    if (peerConnection_.get() == nullptr) {
        std::string msg = "Failed to create peer connection";
        RTC_LOG_F(LS_ERROR) << msg;
        if (listener_) listener_->OnDisconnected(id_);
    }
}

WebRtcConnection::~WebRtcConnection() {
    RTC_LOG_F(LS_VERBOSE);
}

void WebRtcConnection::Send(const rtc::CopyOnWriteBuffer& data, bool binary) {
    if (!listener_) return;
    if (channels_.empty()) {
        RTC_LOG_F(LS_WARNING) << "Send called without opened WebRtcChannel";
        return;
    }
    
    for (unsigned int i = 0; i < channels_.size(); i++) {
        unsigned int selectedChannel = channelSelector_ + i;
        if (selectedChannel >= channels_.size()) {
            selectedChannel = 0;
        }
        ChannelRefPtr channel = channels_.at(selectedChannel);
        if (channel->IsOpen()) {
            channel->Send(data, binary);
            channelSelector_ = selectedChannel + 1;
            return;
        }
    }
}

void WebRtcConnection::CreateOffer() {
    if (!peerConnection_.get()) {
        RTC_LOG_F(LS_WARNING) << "CreateOffer called without created peer connection";
        return;
    }
    for (unsigned int i = 0; i < 1; i++) {
        CreateChannel();
    }

    peerConnection_->CreateOffer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}

void WebRtcConnection::CreateChannel() {
    if (!peerConnection_.get()) {
        RTC_LOG_F(LS_WARNING) << "CreateChannel called without created peer connection";
        return;
    }
    ChannelRefPtr channel = channelFactory_->Create(peerConnection_.get(), this);
    channels_.push_back(channel);
}

void WebRtcConnection::CreateAnswer() {
    if (!peerConnection_.get()) {
        RTC_LOG_F(LS_WARNING) << "CreateAnswer called without created peer connection";
        return;
    }
    peerConnection_->CreateAnswer(this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}

void WebRtcConnection::SetRemoteDescription(webrtc::SessionDescriptionInterface* sessionDescription) {
    if (!peerConnection_.get()) {
        RTC_LOG_F(LS_WARNING) << "SetRemoteDescription called without created peer connection";
        return;
    }
    peerConnection_->SetRemoteDescription(this, sessionDescription);
}

void WebRtcConnection::AddIceCandidate(webrtc::IceCandidateInterface* candidate) {
    if (!peerConnection_.get()) {
        RTC_LOG_F(LS_WARNING) << "AddIceCandidate called without created peer connection";
        return;
    }
    if (peerConnection_->remote_description() && peerConnection_->local_description()) {
        if (peerConnection_->AddIceCandidate(candidate)) {
            delete candidate;
        } else {
            candidates_.push_back(std::shared_ptr<webrtc::IceCandidateInterface>(candidate));
        }
    } else {
        candidates_.push_back(std::shared_ptr<webrtc::IceCandidateInterface>(candidate));
    }
}

void WebRtcConnection::Close() {
    listener_ = nullptr;
    for (Channels::iterator it = channels_.begin(); it != channels_.end(); ++it) {
        ChannelRefPtr channel = *it;
        if (channel.get()) {
            channel->Close();
            rtc::Thread::Current()->Dispose(new RefPtrDisposer<WebRtcChannel>(channel));
        }
    }
    channels_.clear();

    if (peerConnection_.get()) {
        rtc::Thread::Current()->Dispose(new RefPtrDisposer<webrtc::PeerConnectionInterface>(peerConnection_));
        peerConnection_ = nullptr;
    }
}

std::string WebRtcConnection::GetId() const {
    return id_;
}

void WebRtcConnection::RequestStatistic() {
    if (!peerConnection_.get()) {
        RTC_LOG_F(LS_WARNING) << "RequestStatistic called without created peer connection";
        return;
    }
    peerConnection_->GetStats(this);
}

void WebRtcConnection::DrainCandidates() {
    for (Candidates::iterator it = candidates_.begin(); it != candidates_.end(); ++it) {
        CandidateRefPtr candidate = *it;
        peerConnection_->AddIceCandidate(candidate.get());
        candidate.reset();
    }
    candidates_.clear();
}

// PeerConnectionObserver interface
void WebRtcConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
    ChannelRefPtr ch = channelFactory_->Create(channel, this);
    channels_.push_back(ch);
}

void WebRtcConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
    if (new_state == webrtc::PeerConnectionInterface::kIceConnectionFailed ||
        new_state == webrtc::PeerConnectionInterface::kIceConnectionClosed) {
        RTC_LOG_F(LS_INFO) << "WebRtcConnection::OnIceConnectionChange( " << new_state << " ) : \n 4-failed, 5-disconnected, 6-closed";
        if (listener_) {
            WebRtcConnectionListener* listener = listener_;
            listener_ = nullptr;
            listener->OnDisconnected(id_);
        }
    }
}

void WebRtcConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    if (candidate == nullptr) {
        RTC_LOG_F(LS_WARNING) << "Received null candidate";
        return;
    }
    
    RTC_LOG_F(LS_INFO) << "WebRtcConnection::OnIceCandidate " << candidate->sdp_mline_index();
    
    std::string sdp;
    if (!candidate->ToString(&sdp)) {
        std::string msg = "Failed to serialize candidate";
        RTC_LOG_F(LS_WARNING) << msg;
        return;
    }

    if (listener_) listener_->OnCandidate(id_.c_str(), candidate->sdp_mid().c_str(), candidate->sdp_mline_index(), sdp.c_str());
}

void WebRtcConnection::OnIceConnectionReceivingChange(bool receiving) {
    RTC_LOG_F(LS_VERBOSE) << "WebRtcConnection::OnIceConnectionReceivingChange";
}

// RTCStatsCollectorCallback interface
void WebRtcConnection::OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport> &report) {
    std::string statistic = report->ToJson();
    RTC_LOG_F(LS_VERBOSE) << "STATS " << statistic;
    if (listener_) listener_->OnStatistic(id_, statistic);
}

// CreateSessionDescriptionObserver interface
void WebRtcConnection::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    if (!peerConnection_.get()) {
        std::string msg = "SetLocalDescription called without created peer connection";
        RTC_LOG_F(LS_WARNING) << msg;
        return;
    }
    peerConnection_->SetLocalDescription(this, desc);

    std::string sdp;
    desc->ToString(&sdp);

    if (listener_) listener_->OnLocalDescription(id_.c_str(), desc->type().c_str(), sdp.c_str());
}

// SetSessionDescriptionObserver interface
void WebRtcConnection::OnSuccess() {
    if (peerConnection_->remote_description()) {
        if (peerConnection_->local_description()) {
            DrainCandidates();
        } else {
            CreateAnswer();
        }
    }
    
}

// CreateSessionDescriptionObserver and SetSessionDescriptionObserver interface
void WebRtcConnection::OnFailure(const std::string& error) {
    std::string msg = "Session description failed, error: ";
    msg.append(error);
    RTC_LOG_F(LS_WARNING) << msg;
}

// WebRtcChannelListener interface
void WebRtcConnection::OnOpened() {
    if (listener_ && !channelsOpened_) listener_->OnConnected(id_);
    channelsOpened_++;
}

void WebRtcConnection::OnClosed() {
    channelsOpened_--;
    
    if (listener_ && !channelsOpened_) {
        WebRtcConnectionListener* listener = listener_;
        listener_ = nullptr;
        listener->OnDisconnected(id_);
    }
}

void WebRtcConnection::OnMessage(std::string message) {
    if (listener_) listener_->OnMessage(id_, message);
    else RTC_LOG_F(LS_WARNING) << "Received connection message, but listener is nullptr";
}

void WebRtcConnection::OnBufferedAmountChanged() {
    if (listener_) {
        uint64_t bufferedAmount = 0;
        for (Channels::iterator it = channels_.begin(); it != channels_.end(); ++it) {
            ChannelRefPtr channel = *it;
            bufferedAmount += channel->GetBufferedAmount();
        }
        listener_->OnBufferedAmountChanged(id_, bufferedAmount);
    }
}


// rtc::RefCountInterface interface
void WebRtcConnection::AddRef() const {
    rtc::AtomicOps::Increment(&ref_count_);
}
rtc::RefCountReleaseStatus WebRtcConnection::Release() const {
    int count = rtc::AtomicOps::Decrement(&ref_count_);
    
    if (!count) {
        delete this;
        return rtc::RefCountReleaseStatus::kDroppedLastRef;
    }
    //return count;
    return rtc::RefCountReleaseStatus::kOtherRefsRemained;
}
