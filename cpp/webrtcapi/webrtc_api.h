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

#ifndef webrtcapi_
#define webrtcapi_

#ifdef _WIN32
    #ifdef ERROR
        #undef ERROR
    #endif
#endif

struct WebRtcListener {
    virtual ~WebRtcListener() {}
    virtual void on_connected(const char* connection_id) = 0;
    virtual void on_disconnected(const char* connection_id) = 0;
    virtual void on_message(const char* connection_id, const char* data, unsigned long long size) = 0;
    virtual void on_buffered_amount_change(const char* connection_id, unsigned long long newValue) = 0;
    virtual void on_local_description(const char* connection_id, const char* type, const char* sdp) = 0;
    virtual void on_candidate(const char* connection_id,
                              const char* sdp_mid,
                              int sdp_m_line_index,
                              const char* candidate) = 0;
    virtual void on_statistic(const char* connection_id,
                              const char *statistic,
                              unsigned long long size) = 0;
    
};

class WebRtcProxy;
class WebRtcProxyFactory;


class WebRtc {
public:
#ifdef ANDROID
    static void AndroidGlobalInitialize(void* ctx, void* jvm);
#endif
    
    enum LogLevel {
        SENSITIVE,
        VERBOSE,
        INFO,
        WARNING,
        ERROR,
        NONE
    };
    
    static void set_log_level(LogLevel level);
    
    WebRtc(WebRtcProxyFactory* factory = nullptr);
    WebRtc(const WebRtc& other);
    virtual ~WebRtc();

    void set_listener(WebRtcListener* listener);

    void add_ice_server(const char* uri, const char* username, const char* password);    
    
    void create_connection(const char* connection_id);
    
    void initiate_connection(const char* connection_id);
    void set_remote_description(const char* connection_id, const char* type, const char* sdp);
    void set_candidate(const char* connection_id, const char* sdp_mid, int sdp_m_line_index, const char* candidate);
    
    void disconnect(const char* connection_id);
    
    void send(const char* connection_id, const char* data, long size, bool binary);
    
    void request_statistic(const char* connection_id);

    void close();
    
private:
    WebRtcProxy* proxy_;
};
#endif
