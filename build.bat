call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
mkdir ./build
cmake -G "Visual Studio 16 2019" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S . -B ./build
cd ./build
nmake
