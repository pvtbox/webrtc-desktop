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

#ifndef WEBRTC_PROXY_H
#define WEBRTC_PROXY_H
#pragma once

#ifdef ANDROID
#include <jni.h>
#endif

#include <string>
#include <atomic>

#include <rtc_base/scoped_ref_ptr.h>
#include <rtc_base/messagehandler.h>

#include "webrtc_api.h"
#include "WebRtcConnectionPoolListener.h"

namespace rtc {
    class Thread;
}

class WebRtcConnectionPool;
class WebRtcConnectionPoolFactory;
namespace {
    class WebRtcProxyMessage;
}

class WebRtcProxy : public WebRtcConnectionPoolListener,
                    public rtc::MessageHandler {
public:
    WebRtcProxy(WebRtcConnectionPoolFactory* poolFactory = nullptr);
    virtual ~WebRtcProxy();
    
    virtual void set_listener(WebRtcListener* listener);

    virtual void add_ice_server(std::string uri, std::string username, std::string password);
    
    virtual void create_connection(std::string connection_id);
                        
    virtual void initiate_connection(std::string connection_id);
    virtual void set_remote_description(std::string connection_id, std::string type, std::string sdp);
    virtual void set_candidate(std::string connection_id, std::string sdp_mid, int sdp_m_line_index, std::string candidate);
    
    virtual void disconnect(std::string connection_id);
    
    virtual void send(std::string connection_id, const char* data, size_t size, bool binary);
                        
    virtual void request_statistic(std::string connection_id);

#ifdef ANDROID
    void set_java_vm(JavaVM* jvm);
#endif

protected:
    // WebRtcConnectionPoolListenerInterface
    void OnConnected(std::string id) override;
    void OnDisconnected(std::string id) override;
    void OnMessage(std::string id, std::string message) override;
    void OnBufferedAmountChanged(std::string id, uint64_t amount) override;
    void OnLocalDescription(std::string id, std::string type, std::string sdp) override;
    void OnCandidate(std::string id, std::string sdpMid, int sdpMLineIndex, std::string candidate) override;
    void OnStatistic(std::string id, std::string statistic) override;

    // rtc::MessageHandler interface
    void OnMessage(rtc::Message* msg) override;

private:
    void close();

    void PostToOwnThread(WebRtcProxyMessage* msg);
    void SendToOwnThread(WebRtcProxyMessage* msg);
    void PostToPythonThread(WebRtcProxyMessage* msg);
    void SendToPythonThread(WebRtcProxyMessage* msg);
    
    typedef std::unique_ptr<rtc::Thread> ThreadUniqPtr;
    ThreadUniqPtr ownThread_;
    ThreadUniqPtr pythonThread_;

    WebRtcListener* listener_;

    std::unique_ptr<WebRtcConnectionPoolFactory> poolFactory_;
                        
    typedef rtc::scoped_refptr<WebRtcConnectionPool> ConnectionPoolRefPtr;
    ConnectionPoolRefPtr connectionPool_;

#ifdef ANDROID
    JavaVM* jvm_;
#endif
};

#endif // WEBRTC_PROXY_H
