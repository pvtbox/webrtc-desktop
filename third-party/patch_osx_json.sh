sed -i .original "s/rtc_static_library(\"webrtc_common\") {/rtc_static_library(\"webrtc_common\") {public_deps = [\"\/\/webrtc\/base:rtc_json\",]/" BUILD.gn

