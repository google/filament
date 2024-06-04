#!/bin/bash

# Usage: the first argument selects the build type:
# - release, to build release only
# - debug, to build debug only
# - continuous, to build release and debug
# - presubmit, for presubmit builds
#
# The default is release

echo "This script is intended to run in a CI environment and may modify your current environment."
echo "Please refer to BUILDING.md for more information."

read -r -p "Do you wish to proceed (y/n)? " choice
case "${choice}" in
    y|Y)
	      echo "Build will proceed..."
	      ;;
    n|N)
    	  exit 0
    	  ;;
	  *)
        exit 0
        ;;
esac

set -e
set -x

UNAME=`echo $(uname)`
LC_UNAME=`echo $UNAME | tr '[:upper:]' '[:lower:]'`

FILAMENT_ANDROID_CI_BUILD=true

# build-common.sh will generate the following variables:
#     $GENERATE_ARCHIVES
#     $BUILD_DEBUG
#     $BUILD_RELEASE
source `dirname $0`/../common/ci-common.sh
if [[  "$LC_UNAME" == "linux" ]]; then
    source `dirname $0`/../linux/ci-common.sh
elif [[ "$LC_UNAME" == "darwin" ]]; then
    source `dirname $0`/../mac/ci-common.sh
fi
source `dirname $0`/../common/build-common.sh

if [[ "$GITHUB_WORKFLOW" ]]; then
    java_version=$(java -version 2>&1 | head -1 | cut -d'"' -f2 | sed '/^1\./s///' | cut -d'.' -f1)
    if [[ "$java_version" < 17 ]]; then
        echo "Android builds require Java 17, found version ${java_version} instead"
        exit 0
    fi
fi

# Unless explicitly specified, NDK version will be set to match exactly the required one
FILAMENT_NDK_VERSION=${FILAMENT_NDK_VERSION:-$(cat `dirname $0`/ndk.version)}

# Install the required NDK version specifically (if not present)
if [[ ! -d "${ANDROID_HOME}/ndk/$FILAMENT_NDK_VERSION" ]]; then
    ${ANDROID_HOME}/cmdline-tools/latest/bin/sdkmanager "ndk;$FILAMENT_NDK_VERSION"
fi

# Only build 1 64 bit target during presubmit to cut down build times during presubmit
# Continuous builds will build everything
ANDROID_ABIS=
if [[ "$TARGET" == "presubmit" ]]; then
  ANDROID_ABIS="-q arm64-v8a"
fi

# Build the Android sample-gltf-viewer APK during release.
BUILD_SAMPLES=
if [[ "$TARGET" == "release" ]]; then
    BUILD_SAMPLES="-k sample-gltf-viewer"
fi

pushd `dirname $0`/../.. > /dev/null
FILAMENT_NDK_VERSION=${FILAMENT_NDK_VERSION} ./build.sh -p android $ANDROID_ABIS -c $BUILD_SAMPLES $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
