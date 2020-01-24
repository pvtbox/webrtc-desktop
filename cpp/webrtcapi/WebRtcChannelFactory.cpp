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

#include "WebRtcChannelFactory.h"
#include "WebRtcChannel.h"

WebRtcChannelFactory::~WebRtcChannelFactory() {
}

rtc::scoped_refptr<WebRtcChannel> WebRtcChannelFactory::Create(webrtc::PeerConnectionInterface* peerConnection,
                                                               WebRtcChannelListener* listener) {
    rtc::scoped_refptr<WebRtcChannel> ch = new rtc::RefCountedObject<WebRtcChannel>(peerConnection, listener);
    ch->InitChannel();
    return ch;
    
}

rtc::scoped_refptr<WebRtcChannel> WebRtcChannelFactory::Create(rtc::scoped_refptr<webrtc::DataChannelInterface> channel,
                                                               WebRtcChannelListener* listener) {
    rtc::scoped_refptr<WebRtcChannel> ch = new rtc::RefCountedObject<WebRtcChannel>(channel, listener);
    ch->InitChannel();
    return ch;
}
