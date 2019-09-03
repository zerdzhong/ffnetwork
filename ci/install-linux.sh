#!/usr/bin/env bash

if [[ "$OSTYPE" != "linux-gnu" ]]; then
    echo "Not a Linux build; skipping installation"
    exit 0;
fi

if ! [ -x "$(command -v cmake)" ]; then
    DEPS_DIR="${TRAVIS_BUILD_DIR}/deps"

    mkdir ${DEPS_DIR} && cd ${DEPS_DIR}

    wget --no-check-certificate https://cmake.org/files/v3.12/cmake-3.12.3-Linux-x86_64.tar.gz
    tar -xvf cmake-3.12.3-Linux-x86_64.tar.gz > /dev/null
    mv cmake-3.12.3-Linux-x86_64 cmake-install
    cp -r cmake-install/*  /usr/local/bin
    PATH=${DEPS_DIR}/cmake-install:${DEPS_DIR}/cmake-install/bin:$PATH
    echo $PATH
    cd ${TRAVIS_BUILD_DIR}
fi

sudo apt install -y gcovr
pip install --user cpp-coveralls
