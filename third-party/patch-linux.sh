find webrtc talk chromium/src/third_party \( -name *.gyp -o  -name *.gypi \) -not -path *libyuv* -exec sed -i "s|\('type': 'static_library',\)|\1 'standalone_static_library': 1,|" '{}' ';'
find chromium/src/third_party/icu/icu.gyp \( -name *.gyp -o  -name *.gypi \) -exec sed -i "s|\('type': 'none',\)|\1 'standalone_static_library': 0,|" '{}' ';'
