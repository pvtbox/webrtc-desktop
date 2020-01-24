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

#ifndef WebRtcChannelMock_h
#define WebRtcChannelMock_h

#include <gmock/gmock.h>

#include "WebRtcChannel.h"

class WebRtcChannelMock: public WebRtcChannel {
public:
    WebRtcChannelMock(): WebRtcChannel() {}
    virtual ~WebRtcChannelMock() { Destructor(); }
    MOCK_METHOD0(Destructor, void());
    MOCK_METHOD0(Close, void());
    MOCK_METHOD2(Send, void(const rtc::CopyOnWriteBuffer&, bool));
    MOCK_METHOD0(IsOpen, bool());
    MOCK_METHOD0(GetBufferedAmount, uint64_t());
};

#endif /* WebRtcChannelMock_h */
