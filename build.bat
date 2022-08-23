rem set CC=clang-cl
rem set CXX=clang-cl
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -S . -B build
ninja -C build
.\build\widgets\Neobox.exe
