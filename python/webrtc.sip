%Module(name=webrtc)


struct WebRtcListener {

%TypeHeaderCode
#include <webrtc_api.h>
%End
    virtual void on_connected(const char* connection_id) = 0;
    virtual void on_disconnected(const char* connection_id) = 0;
    virtual void on_message(const char* connection_id, const char* data, unsigned long long size) = 0;
%VirtualCatcherCode
        sipCallMethod(&sipIsErr, sipMethod, "sg", a0, a1, a2);
%End
        
    virtual void on_buffered_amount_change(const char* connection_id, unsigned long long newValue) = 0;
    virtual void on_local_description(const char* connection_id, const char *type, const char* sdp) = 0;
    virtual void on_candidate(const char* connection_id, const char* sdp_mid, int sdp_m_line_index, const char* candidate) = 0;
    virtual void on_statistic(const char* connection_id, const char* stat, unsigned long long size) = 0;
%VirtualCatcherCode
        sipCallMethod(&sipIsErr, sipMethod, "sg", a0, a1, a2);
%End

};


class WebRtc {

%TypeHeaderCode
#include <webrtc_api.h>
%End

public:
    enum LogLevel {
        SENSITIVE,
        VERBOSE,
        INFO,
        WARNING,
        ERROR,
        NONE
    };
    static void set_log_level(LogLevel level);

    WebRtc();
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
};

