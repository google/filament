#!/bin/bash

# build-common.sh will generate the following variables:
#     $GENERATE_ARCHIVES
#     $BUILD_DEBUG
#     $BUILD_RELEASE

# Typically a build script (build.sh) would source this script. For example,
#   source `dirname $0`/../common/build-common.sh

# Usage: the first argument selects the build type:
# - release, to build release only
# - debug, to build debug only
# - continuous, to build release and debug
# - presubmit, for presubmit builds
#
# The default is release
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
