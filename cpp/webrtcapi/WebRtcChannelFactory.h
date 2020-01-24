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

#ifndef WebRtcChannelFactory_h
#define WebRtcChannelFactory_h

#include "rtc_base/scoped_ref_ptr.h"

namespace webrtc {
    class PeerConnectionInterface;
    class DataChannelInterface;
}
class WebRtcChannelListener;
class WebRtcChannel;

class WebRtcChannelFactory {
public:
    virtual ~WebRtcChannelFactory();
    virtual rtc::scoped_refptr<WebRtcChannel> Create(webrtc::PeerConnectionInterface* peerConnection,
                                                    WebRtcChannelListener* listener);
    virtual rtc::scoped_refptr<WebRtcChannel> Create(rtc::scoped_refptr<webrtc::DataChannelInterface> channel,
                                                    WebRtcChannelListener* listener);
};

#endif /* WebRtcChannelFactory_h */
