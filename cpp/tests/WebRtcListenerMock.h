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

#ifndef WebRtcListenerMock_h
#define WebRtcListenerMock_h

#include <gmock/gmock.h>

#include "webrtc_api.h"

class WebRtcListenerMock: public WebRtcListener {
public:
    MOCK_METHOD1(on_connected, void(const char*));
    MOCK_METHOD1(on_disconnected, void(const char*));
    MOCK_METHOD3(on_message, void(const char*, const char*, unsigned long long));
    MOCK_METHOD2(on_buffered_amount_change, void(const char*, unsigned long long));
    MOCK_METHOD3(on_local_description, void(const char*, const char*, const char*));
    MOCK_METHOD4(on_candidate, void(const char*, const char*, int, const char*));
    MOCK_METHOD3(on_statistic, void(const char*, const char*, unsigned long long));
};

#endif /* WebRtcListenerMock_h */
