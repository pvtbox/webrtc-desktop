#set -Wno-unused-result flag for linux build
find build/config/compiler/BUILD.gn -exec sed -i 's/\"-Wno-unused-parameter\",/\"-Wno-unused-parameter\", \"-Wno-unused-result\",/' '{}' ';'

