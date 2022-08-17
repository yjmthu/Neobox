call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -S . -B build
ninja -C build
