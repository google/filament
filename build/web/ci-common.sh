#!/bin/bash

curl -OL https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-mac.zip
unzip -q ninja-mac.zip
chmod +x ninja
export PATH="$PWD:$PATH"

# FIXME: kokoro machines have node and npm but currently they are symlinked to non-existent files
# npm install -g typescript

# Install emscripten.
curl -L https://github.com/emscripten-core/emsdk/archive/a77638d.zip > emsdk.zip
unzip emsdk.zip ; mv emsdk-* emsdk ; cd emsdk
python emsdk install latest
python emsdk activate latest

export EMSDK="$PWD"
cd ..
