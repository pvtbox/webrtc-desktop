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

#ifndef WEBRTC_CONNECTION_H_
#define WEBRTC_CONNECTION_H_
#pragma once

#include "api/peerconnectioninterface.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "rtc_base/copyonwritebuffer.h"

#include "WebRtcChannelListener.h"


namespace webrtc {
    class PeerConnectionFactoryInterface;
}

class WebRtcConnectionListener;
class WebRtcChannelFactory;
class WebRtcChannel;

class WebRtcConnection : public WebRtcChannelListener,
                         public webrtc::PeerConnectionObserver,
                         public webrtc::RTCStatsCollectorCallback,
                         virtual public webrtc::CreateSessionDescriptionObserver,
                         virtual public webrtc::SetSessionDescriptionObserver {
public:
    typedef std::vector<webrtc::PeerConnectionInterface::IceServer> IceServers;
    WebRtcConnection(std::string id,
                     WebRtcConnectionListener* listener,
                     WebRtcChannelFactory* channelFactory = nullptr);
    virtual void Init(webrtc::PeerConnectionFactoryInterface* factory, IceServers ice);
    virtual void Send(const rtc::CopyOnWriteBuffer& data, bool binary);
    virtual void CreateOffer();
    virtual void SetRemoteDescription(webrtc::SessionDescriptionInterface* sessionDescription);
    virtual void AddIceCandidate(webrtc::IceCandidateInterface* candidate);
    virtual void Close();
    virtual std::string GetId() const;
    virtual void RequestStatistic();

    // rtc::RefCountInterface interface
    void AddRef() const override;
    rtc::RefCountReleaseStatus Release() const override;

protected:
    ~WebRtcConnection();

    // PeerConnectionObserver interface
    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnIceConnectionReceivingChange(bool receiving) override;

    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}
    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
    void OnRenegotiationNeeded() override {}
    void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}
    void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {}
                             
    // RTCStatsCollectorCallback interface
    void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override;

    // CreateSessionDescriptionObserver interface
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    // SetSessionDescriptionObserver interface
    void OnSuccess() override;
    // CreateSessionDescriptionObserver and SetSessionDescriptionObserver interface
    void OnFailure(const std::string& error) override;

    // WebRtcChannelListener interface
    void OnOpened() override;
    void OnClosed() override;
    void OnMessage(std::string message) override;
    void OnBufferedAmountChanged() override;

private:
    void CreateChannel();
    void CreateAnswer();
    void DrainCandidates();

    typedef rtc::scoped_refptr<webrtc::PeerConnectionInterface> PeerConnectionRefPtr; 
    PeerConnectionRefPtr peerConnection_;
                             
    typedef std::shared_ptr<webrtc::IceCandidateInterface> CandidateRefPtr;
    typedef std::vector<CandidateRefPtr> Candidates;
    Candidates candidates_;
                             
    typedef rtc::scoped_refptr<WebRtcChannel> ChannelRefPtr;
    typedef std::vector<ChannelRefPtr> Channels;
    Channels channels_;
    unsigned int channelSelector_;
    unsigned int channelsOpened_;


    std::string id_;
    WebRtcConnectionListener* listener_;
    std::unique_ptr<WebRtcChannelFactory> channelFactory_;

    mutable volatile int ref_count_ = 0;
};

#endif  // WEBRTC_CONNECTION_H_
