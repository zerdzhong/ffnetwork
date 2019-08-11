#!/usr/bin/env sh

set -e

cmake --version
mkdir build || true
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=1
make
CTEST_OUTPUT_ON_FAILURE=1 make unittest
cd unittest
ctest --verbose
make ffnetwork_coverage
