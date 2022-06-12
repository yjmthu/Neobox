#!/bin/bash

SCRIPT_DIR=$(cd $(dirname $0);pwd)
BUILD_DIR="${SCRIPT_DIR}/build"
CMAKE_BUILD_TYPE="Release"
cd ${SCRIPT_DIR}

if [ ! -f "CMakeLists.txt" ]; then
    echo "CMakeLists.txt Not Found!"
    exit 1
fi

if [ $# -gt 0 ]; then
    for ((i=1; i<$#; i+=1)); do
        echo $($i)
    done
        echo "---------------"
    if [ "$1" = "-r" ]; then
        CMAKE_BUILD_TYPE="Release"
        echo "Release Build."
    elif [ "$1" = '-k' ]; then
        CMAKE_BUILD_TYPE="$(cat .tmp.txt)"
        echo "${CMAKE_BUILD_TYPE} Build."
    elif [ "$1" = '-d' ]; then
        CMAKE_BUILD_TYPE="Debug"
        echo "Debug Build."
    fi
else
    echo "Build Only."
fi

if [ "$(cat .tmp.txt)" != "$CMAKE_BUILD_TYPE" ]; then
    rm -rf ${BUILD_DIR}
fi

echo "$CMAKE_BUILD_TYPE" > .tmp.txt

if [ ! -d "${BUILD_DIR}" ]; then
    mkdir ${BUILD_DIR}
fi

cd ${BUILD_DIR}

if [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    echo "================== linux ===================="
    cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -G Ninja -S ..
    ninja
    if [ -f "${BUILD_DIR}/widgets/widgets" ]; then
        ${BUILD_DIR}/widgets/widgets &
    fi
else
    echo "================== windows =================="
    VS_2022="C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build"
    VS_2022_X64="${VS_2022}\\vcvars64.bat"
    VS_2022_X64_X86="${VS_2022}\\vcvarsamd64_x86.bat"
    VS_2022_X86="${VS_2022}\\vcvars32.bat"
    VS_2022_X86_X64="${VS_2022}\\vcvarsx86_amd64.bat"
    VS_NOW="\"${VS_2022_X64}\" & set CC=cl & set CXX=cl & cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -G Ninja -S .. & ninja"
    echo ${VS_NOW}
    cmd.exe /c "${VS_NOW}"
    if [ -f "${BUILD_DIR}/widgets/widgets.exe" ]; then
        ${BUILD_DIR}/widgets/widgets.exe &
    fi
fi
