#!/usr/bin/env bash

set -e

cmake --version
mkdir build || true
cd build

if [ "$TRAVIS_OS_NAME" = 'osx' ]; then
  cmake .. -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=1 -DUNITTEST_TIME_INSENSITIVE=1
else
  cmake .. -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=1
fi

make -j 6
CTEST_OUTPUT_ON_FAILURE=1 make unittest
cd unittest
ctest --verbose

cd -
cd ./source/base/tests/
ctest --verbose

set +e
#disable exit on non 0
make ffnetwork_coverage || true
echo "finish"
