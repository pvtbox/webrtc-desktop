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
#include <cassert>
#include "rtc_base/criticalsection.h"
#include "rtc_base/logging.h"

#ifdef ANDROID
#include "webrtc/modules/utility/include/jvm_android.h"
#include "webrtc/sdk/android/src/jni/classreferenceholder.h"
#include "webrtc/sdk/android/src/jni/jni_helpers.h"
#include "webrtc/sdk/android/src/jni/androidnetworkmonitor_jni.h"
#endif

#include "webrtc_api.h"
#include "WebRtcProxyFactory.h"
#include "WebRtcProxy.h"


namespace {
    static rtc::CriticalSection cs_;
}

#ifdef ANDROID
static bool initialized = false;
static JavaVM* javaVM = nullptr;

void WebRtc::AndroidGlobalInitialize(void* ctx, void* j) {
    cs_.Enter();
    RTC_LOG_F(LS_VERBOSE);
    if (initialized) return;
    jobject context = (jobject)ctx;
    assert(context != NULL);
    JNIEnv* jenv = (JNIEnv*)j;
    assert(jenv != NULL);
    JavaVM* jvm = NULL;
    assert(jvm == NULL);
    assert(jenv->GetJavaVM(&jvm) == 0);
    assert(jvm != NULL);

    assert(webrtc_jni::InitGlobalJniVariables(jvm) > 0);
    assert(jvm != NULL);
    assert(jenv != NULL);
    assert(webrtc_jni::GetJVM() != NULL);
    assert(webrtc_jni::GetEnv() != NULL);
    webrtc_jni::AndroidNetworkMonitor::SetAndroidContext(webrtc_jni::GetEnv(), context);
	webrtc::JVM::Initialize(jvm, context);
    javaVM = jvm;
    initialized = true;
    cs_.Leave();
}
#endif


WebRtc::WebRtc(WebRtcProxyFactory* factory) {
    factory = factory != nullptr ? factory : new WebRtcProxyFactory();
    proxy_ = factory->Create();
    delete factory;
#ifdef ANDROID
    proxy_->set_java_vm(javaVM);
#endif
    assert(proxy_ != nullptr);
}

WebRtc::WebRtc(const WebRtc& other) : proxy_(new WebRtcProxy()) {
}

WebRtc::~WebRtc() {
    cs_.Enter();
    RTC_LOG_F(LS_VERBOSE);
    if (proxy_) {
        delete proxy_;
        proxy_ = nullptr;
    }
    cs_.Leave();
}

void WebRtc::set_listener(WebRtcListener* listener) {
	if(proxy_) proxy_->set_listener(listener);
}

void WebRtc::add_ice_server(const char* uri, const char* username, const char* password) {
    if(proxy_) proxy_->add_ice_server(uri, username, password);
}

void WebRtc::create_connection(const char* connection_id) {
    if(proxy_) return proxy_->create_connection(connection_id);
}

void WebRtc::initiate_connection(const char* connection_id) {
    if(proxy_) return proxy_->initiate_connection(connection_id);
}

void WebRtc::set_remote_description(const char* connection_id, const char* type, const char* sdp) {
    if(proxy_) return proxy_->set_remote_description(connection_id, type, sdp);
}

void WebRtc::set_candidate(const char* connection_id, const char* sdp_mid, int sdp_m_line_index, const char* candidate) {
    if(proxy_) return proxy_->set_candidate(connection_id, sdp_mid, sdp_m_line_index, candidate);
}

void WebRtc::disconnect(const char* connection_id) {
    if(proxy_) proxy_->disconnect(connection_id);
}

void WebRtc::send(const char* connection_id, const char* data, long size, bool binary) {
    if(proxy_) proxy_->send(connection_id, data, size, binary);
}

void WebRtc::request_statistic(const char* connection_id) {
    if (proxy_) proxy_->request_statistic(connection_id);
}

void WebRtc::close() {
    cs_.Enter();
    if (proxy_) {
        delete proxy_;
        proxy_ = nullptr;
    }
    cs_.Leave();
}

void WebRtc::set_log_level(WebRtc::LogLevel level) {
    rtc::LogMessage::LogThreads();
    rtc::LogMessage::LogTimestamps();
    rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)level);
}
