#!/bin/bash

curl -OL https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-mac.zip
unzip -q ninja-mac.zip
chmod +x ninja
export PATH="$PWD:$PATH"

# FIXME: kokoro machines have node and npm but currently they are symlinked to non-existent files
# npm install -g typescript

# Install emscripten.
curl -L https://github.com/emscripten-core/emsdk/archive/1.39.19.zip > emsdk.zip
unzip emsdk.zip ; mv emsdk-* emsdk ; cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

export EMSDK="$PWD"
cd ..
