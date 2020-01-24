mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
cmake --build . --target artifacts
cd artifacts
