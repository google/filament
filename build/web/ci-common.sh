#!/bin/bash
if [ `uname` == "Linux" ];then
    curl -OL https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip
    unzip -q ninja-linux.zip
elif [ `uname` == "Darwin" ];then
    curl -OL https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-mac.zip
    unzip -q ninja-mac.zip
else
    echo "Unsupported OS"
    exit 1
fi

chmod +x ninja
export PATH="$PWD:$PATH"

# FIXME: kokoro machines have node and npm but currently they are symlinked to non-existent files
# npm install -g typescript

# Install emscripten.
curl -L https://github.com/emscripten-core/emsdk/archive/refs/tags/3.1.15.zip > emsdk.zip
unzip emsdk.zip ; mv emsdk-* emsdk ; cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

export EMSDK="$PWD"
cd ..
