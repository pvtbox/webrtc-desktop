@md build
@cd build
@call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
@cmake -G "Visual Studio 14 2015" .. -DCMAKE_BUILD_TYPE=release
@cmake --build . --target artifacts
@cd ..
