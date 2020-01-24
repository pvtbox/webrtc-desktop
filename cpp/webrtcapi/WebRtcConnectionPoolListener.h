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

#ifndef WEBRTC_CONNECTION_POOL_LISTENER_H_
#define WEBRTC_CONNECTION_POOL_LISTENER_H_
#pragma once

#include <string>


class WebRtcConnectionPoolListener {
public:
    virtual void OnConnected(std::string id) = 0;
    virtual void OnDisconnected(std::string id) = 0;
    virtual void OnMessage(std::string id, std::string message) = 0;
    virtual void OnBufferedAmountChanged(std::string id, uint64_t amount) = 0;
    virtual void OnLocalDescription(std::string id, std::string type, std::string sdp) = 0;
    virtual void OnCandidate(std::string id, std::string sdpMid, int sdpMLineIndex, std::string candidate) = 0;
    virtual void OnStatistic(std::string id, std::string statistic) = 0;
};


#endif  // WEBRTC_CONNECTION_POOL_LISTENER_H_
