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

#include <cassert>
#include "WebRtcConnectionFactory.h"
#include "RefPtrDisposer.h"

WebRtcConnectionFactory::WebRtcConnectionFactory(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory) :
  factory_(factory) {
      if (factory_ != nullptr) {
          webrtc::PeerConnectionFactoryInterface::Options options;
          options.disable_network_monitor = true;
          options.disable_sctp_data_channels = false;
          options.disable_encryption = false;
          factory_->SetOptions(options);
      }
}

WebRtcConnectionFactory::~WebRtcConnectionFactory() {
    rtc::Thread::Current()->Dispose(new RefPtrDisposer<webrtc::PeerConnectionFactoryInterface>(factory_));
    factory_ = nullptr;
}

rtc::scoped_refptr<WebRtcConnection> WebRtcConnectionFactory::Create(std::string id,
                                                                     WebRtcConnection::IceServers ice,
                                                                     WebRtcConnectionListener* listener) {
    assert(factory_ != nullptr);
    rtc::scoped_refptr<WebRtcConnection> conn = new WebRtcConnection(id, listener);
    conn->Init(factory_.get(), ice);
    return conn;
}
