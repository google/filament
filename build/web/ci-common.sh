#!/bin/bash

curl -OL https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-mac.zip
unzip -q ninja-mac.zip
chmod +x ninja
export PATH="$PWD:$PATH"

curl -L https://github.com/juj/emsdk/archive/0d8576c.zip > emsdk.zip
unzip emsdk.zip
mv emsdk-* emsdk
emsdk/emsdk update
emsdk/emsdk install sdk-1.38.11-64bit
emsdk/emsdk activate sdk-1.38.11-64bit
export EMSDK="$PWD/emsdk"
