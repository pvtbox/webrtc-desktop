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

#ifndef WebRtcChannelListenerMock_h
#define WebRtcChannelListenerMock_h

#include <gmock/gmock.h>

#include "WebRtcChannelListener.h"

class WebRtcChannelListenerMock: public WebRtcChannelListener {
public:
    MOCK_METHOD0(OnOpened, void());
    MOCK_METHOD0(OnClosed, void());
    MOCK_METHOD1(OnMessage, void(std::string));
    MOCK_METHOD0(OnBufferedAmountChanged, void());
    MOCK_METHOD1(OnError, void(std::string));
    MOCK_METHOD1(OnFatalError, void(std::string));
};

#endif /* WebRtcChannelListenerMock_h */
