#!/bin/bash

if [[ "$GITHUB_WORKFLOW" ]]; then
    OS_NAME=$(uname -s)
    NINJA_DL_DIR=/tmp/ninja-dl
    NINJA_PLATFORM=
    if [[ "$OS_NAME" == "Linux" ]]; then
        NINJA_PLATFORM=linux
    elif [[ "$OS_NAME" == "Darwin" ]]; then
        NINJA_PLATFORM=mac
    fi
    curl -L -o /tmp/ninja-dl.zip https://github.com/ninja-build/ninja/releases/download/v${GITHUB_NINJA_VERSION}/ninja-${NINJA_PLATFORM}.zip
    mkdir -p $NINJA_DL_DIR
    unzip -q /tmp/ninja-dl.zip -d $NINJA_DL_DIR
    chmod +x ${NINJA_DL_DIR}/ninja
    # Install ninja globally for AGP
    sudo cp ${NINJA_DL_DIR}/ninja /usr/local/bin/
fi
