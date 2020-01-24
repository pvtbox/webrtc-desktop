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

#ifndef DataChannelInterfaceMock_h
#define DataChannelInterfaceMock_h

#include <gmock/gmock.h>

#include "webrtc/api/datachannelinterface.h"

class DataChannelInterfaceMock: public webrtc::DataChannelInterface {
public:
    virtual ~DataChannelInterfaceMock() { Destructor(); }
    MOCK_METHOD0(Destructor, void());
    MOCK_METHOD1(RegisterObserver, void(webrtc::DataChannelObserver*));
    MOCK_METHOD0(UnregisterObserver, void());
    MOCK_CONST_METHOD0(label, std::string());
    MOCK_CONST_METHOD0(reliable, bool());
    MOCK_CONST_METHOD0(id, int());
    MOCK_CONST_METHOD0(state, webrtc::DataChannelInterface::DataState());
    MOCK_CONST_METHOD0(buffered_amount, uint64_t());
    MOCK_CONST_METHOD0(messages_sent, uint32_t());
    MOCK_CONST_METHOD0(bytes_sent, uint64_t());
    MOCK_CONST_METHOD0(messages_received, uint32_t());
    MOCK_CONST_METHOD0(bytes_received, uint64_t());
    MOCK_METHOD0(Close, void());
    MOCK_METHOD1(Send, bool(const webrtc::DataBuffer&));
};

#endif /* DataChannelInterfaceMock_h */
