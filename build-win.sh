mkdir -p build
cd build
C:/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio\ 14.0/VC/vcvarsall.bat && cmake -G 'Visual Studio 14 2015' .. -DCMAKE_BUILD_TYPE=RELEASE
cmake --build . --target artifacts
cd artifacts
