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

#ifndef WEBRTC_CHANNEL_H_
#define WEBRTC_CHANNEL_H_
#pragma once

#include "rtc_base/scoped_ref_ptr.h"
#include "rtc_base/copyonwritebuffer.h"
#include "api/datachannelinterface.h"


namespace webrtc {
    class PeerConnectionInterface;
}

class WebRtcChannelListener;

class WebRtcChannel : public webrtc::DataChannelObserver,
                      public rtc::RefCountInterface {
public:
    typedef rtc::scoped_refptr<webrtc::DataChannelInterface> DataChannelRefPtr;
    WebRtcChannel(webrtc::PeerConnectionInterface* peerConnection, WebRtcChannelListener* listener);
    WebRtcChannel(DataChannelRefPtr channel, WebRtcChannelListener* listener);
    void InitChannel();

    virtual void Close();
    virtual void Send(const rtc::CopyOnWriteBuffer& data, bool binary);
                          
    virtual bool IsOpen();
    virtual uint64_t GetBufferedAmount();

protected:
    WebRtcChannel();
    ~WebRtcChannel();

    // DataChannelObserver interface
    void OnMessage(const webrtc::DataBuffer& buffer) override;
    void OnStateChange() override;
    void OnBufferedAmountChange(uint64_t previous_amount) override;

private:
    DataChannelRefPtr channel_;

    WebRtcChannelListener* listener_;
};

#endif  // WEBRTC_CHANNEL_H_
