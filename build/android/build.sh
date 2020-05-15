#!/bin/bash

# Usage: the first argument selects the build type:
# - release, to build release only
# - debug, to build debug only
# - continuous, to build release and debug
# - presubmit, for presubmit builds
#
# The default is release

NDK_VERSION="ndk;21.0.6113669"
ANDROID_NDK_VERSION=21

# Exclude Vulkan from CI builds for Android. (It is enabled for other platforms.)
EXCLUDE_VULKAN=-v

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

# Only update and install the NDK if necessary, as this can be slow
ndk_side_by_side="${ANDROID_HOME}/ndk/"
if [[ -d $ndk_side_by_side ]]; then
    ndk_version=`ls ${ndk_side_by_side} | sort -V | tail -n 1 | cut -f 1 -d "."`
    if [[ ${ndk_version} -lt ${ANDROID_NDK_VERSION} ]]; then
        ${ANDROID_HOME}/tools/bin/sdkmanager "${NDK_VERSION}" > /dev/null
    fi
else
    ${ANDROID_HOME}/tools/bin/sdkmanager "${NDK_VERSION}" > /dev/null
fi

# Only build 1 32 bit and 1 64 bit target during presubmit to cut down build times
# Continuous builds will build everything
ANDROID_ABIS=
if [[ "$TARGET" == "presubmit" ]]; then
  ANDROID_ABIS="-q arm64-v8a,x86"
fi

pushd `dirname $0`/../.. > /dev/null
./build.sh -p android $EXCLUDE_VULKAN $ANDROID_ABIS -c $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
