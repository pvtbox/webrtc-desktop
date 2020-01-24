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

#ifndef WebRtcConnectionPoolFactoryMock_h
#define WebRtcConnectionPoolFactoryMock_h

#include <gmock/gmock.h>

#include "WebRtcConnectionPoolFactory.h"
#include "WebRtcConnectionPoolMock.h"

using ::testing::_;
using ::testing::Return;

class WebRtcConnectionPoolFactoryMock: public WebRtcConnectionPoolFactory {
public:
    WebRtcConnectionPoolFactoryMock() {
        ON_CALL(*this, Create(_,_))
        .WillByDefault(Return(new rtc::RefCountedObject<testing::NiceMock<WebRtcConnectionPoolMock> >(nullptr, nullptr)));
    }
    ~WebRtcConnectionPoolFactoryMock() { Destructor(); }
    MOCK_METHOD2(Create, WebRtcConnectionPool*(SignalServerClient*, WebRtcConnectionPoolListener*));
    MOCK_METHOD0(Destructor, void());
};

#endif /* WebRtcConnectionPoolFactoryMock_h */
