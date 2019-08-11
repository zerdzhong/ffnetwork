#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" != "osx" ]; then
    echo "Not a osx build; skipping installation"
    exit 0;
fi

brew update

brew upgrade cmake

if brew ls --versions pyenv > /dev/null; then
  pyenv --version
else
  brew install pyenv
fi


eval "$(pyenv init -)"
pyenv install 2.7.6
pyenv global 2.7.6
pyenv rehash
pip install cpp-coveralls
pyenv rehash


if brew ls --versions lcov > /dev/null; then
  lcov --version
else
  brew install lcov
fi

