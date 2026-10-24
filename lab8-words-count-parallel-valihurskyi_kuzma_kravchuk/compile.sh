#!/usr/bin/bash -x

rm -rf build
mkdir build
cd build

cmake_options="-DCMAKE_BUILD_TYPE=Release"

cmake $cmake_options ..
cmake --build . --config Release