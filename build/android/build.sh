#!/bin/bash

# Usage: the first argument selects the build type:
# - release, to build release only
# - debug, to build debug only
# - continuous, to build release and debug
# - presubmit, for presubmit builds
#
# The default is release

set -e
set -x

NDK_VERSION="ndk;20.0.5594570"
ANDROID_NDK_VERSION=20

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
    # Only update and install the NDK if necessary, as this can be slow

    ndk_type=0 # 0 = No SDK, 1 = ndk-bundle, 2 = ndk side-by-side

    ndk_properties="${ANDROID_HOME}/ndk-bundle/source.properties"
    if [[ -f $ndk_properties ]]; then
        ndk_version=`sed -En -e "s/^Pkg.Revision *= *([0-9a-f]+).+/\1/p" ${ndk_properties}`
        if [[ ${ndk_version} -ge ${ANDROID_NDK_VERSION} ]]; then
            ndk_type=1
        fi
    fi

    if [[ ${ndk_type} == 0 ]]; then
        ndk_side_by_side="${ANDROID_HOME}/ndk/"
        if [[ -d $ndk_side_by_side ]]; then
            ndk_version=`ls ${ndk_side_by_side} | sort -V | tail -n 1 | cut -f 1 -d "."`
            if [[ ${ndk_version} -ge ${ANDROID_NDK_VERSION} ]]; then
                ndk_type=2
            fi
        fi
    fi

    if [[ ${ndk_type} == 0 ]]; then
        ${ANDROID_HOME}/tools/bin/sdkmanager "${NDK_VERSION}" > /dev/null
    fi
fi

pushd `dirname $0`/../.. > /dev/null
./build.sh -p android -c $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
