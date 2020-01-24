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

#ifndef WEBRTC_CHANNEL_LISTENER_H_
#define WEBRTC_CHANNEL_LISTENER_H_
#pragma once

#include <string>


class WebRtcChannelListener {
public:
    virtual void OnOpened() = 0;
    virtual void OnClosed() = 0;
    virtual void OnMessage(std::string message) = 0;
    virtual void OnBufferedAmountChanged() = 0;
};


#endif  // WEBRTC_CHANNEL_LISTENER_H_
