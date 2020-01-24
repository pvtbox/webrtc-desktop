find BUILD.gn -exec sed -i 's|rtc_static_library(\"webrtc_common\")\s{|rtc_static_library(\"webrtc_common\") {public_deps = [\"//webrtc/base:rtc_json\",]|' '{}' ';'

