#!/usr/bin/env sh

set -e

cmake --version
mkdir build || true
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=1
make -j 6
CTEST_OUTPUT_ON_FAILURE=1 make unittest
cd unittest
ctest --verbose

set +e
#disable exit on non 0
make ffnetwork_coverage || true
echo "finish"
