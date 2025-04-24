#!/bin/bash

# Install emscripten.
curl -L https://github.com/emscripten-core/emsdk/archive/refs/tags/3.1.60.zip > emsdk.zip
unzip emsdk.zip ; mv emsdk-* emsdk ; cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

export EMSDK="$PWD"
cd ..
