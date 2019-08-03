#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" != "osx" ]; then
    echo "Not a osx build; skipping installation"
    exit 0;
fi