#!/bin/bash

curl -OL https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-mac.zip
unzip -q ninja-mac.zip
chmod +x ninja
# Install ninja globally for AGP
cp ninja /usr/local/bin
export PATH="$PWD:$PATH"
