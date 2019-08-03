#!/usr/bin/env sh

cmake --version
mkdir build || true
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
CTEST_OUTPUT_ON_FAILURE=1 make unittest
cd unittest
ctest --verbose
