#!/bin/bash

if [ -d "./emsdk" ]; then
    echo "emsdk folder found. Assume emsdk has been installed."
    cd emsdk
    ./emsdk activate latest
    source ./emsdk_env.sh
    export EMSDK="$PWD"
    cd ..
    exit 0
fi

# Install emscripten.
EMSDK_VERSION=${GITHUB_EMSDK_VERSION-3.1.60}
curl -L https://github.com/emscripten-core/emsdk/archive/refs/tags/${EMSDK_VERSION}.zip > emsdk.zip
unzip emsdk.zip ; mv emsdk-* emsdk ; cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

export EMSDK="$PWD"
cd ..
