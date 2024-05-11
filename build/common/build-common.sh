#!/bin/bash

if [[ ! "$TARGET" ]]; then
    if [[ "$1" ]]; then
        TARGET=$1
    else
        TARGET=release
    fi
fi

echo "Building $TARGET target"

BUILD_DEBUG=
BUILD_RELEASE=
GENERATE_ARCHIVES=
RUN_TESTS=

if [[ "$TARGET" == "presubmit" ]]; then
    BUILD_RELEASE=release
fi

if [[ "$TARGET" == "debug" ]]; then
    BUILD_DEBUG=debug
    GENERATE_ARCHIVES=-a
fi

if [[ "$TARGET" == "release" ]]; then
    BUILD_RELEASE=release
    GENERATE_ARCHIVES=-a
fi

if [[ "$TARGET" == "continuous" ]]; then
    BUILD_DEBUG=debug
    BUILD_RELEASE=release
    GENERATE_ARCHIVES=-a
    RUN_TESTS=-u
fi
