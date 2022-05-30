#!/bin/sh

# export CC=/usr/bin/clang
# export CXX=/usr/bin/clang++

BUILD_DIR=./build
if [ ! -d "$BUILD_DIR" ]; then
    mkdir $BUILD_DIR
    cmake -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S . -B $BUILD_DIR
fi

cd $BUILD_DIR
ninja
./Neobox &
