#!/bin/bash
if [ `uname` == "Linux" ];then
    source `dirname $0`/../linux/ci-common.sh
elif [ `uname` == "Darwin" ];then
    curl -OL https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-mac.zip
    unzip -q ninja-mac.zip
else
    echo "Unsupported OS"
    exit 1
fi

chmod +x ninja
export PATH="$PWD:$PATH"

# Install emscripten.
curl -L https://github.com/emscripten-core/emsdk/archive/refs/tags/3.1.15.zip > emsdk.zip
unzip emsdk.zip ; mv emsdk-* emsdk ; cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

export EMSDK="$PWD"
cd ..
