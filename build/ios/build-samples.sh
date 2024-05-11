#!/bin/bash

set -ex

source `dirname $0`/../common/build-common.sh

# These flags allow us to build without needing to code sign.
XCODEBUILD_FLAGS='CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED=NO'

# This sets the BUILD_DEBUG and BUILD_RELEASE variables based on the CI job.
pushd `dirname $0`/../../ios/samples

PROJECTS="gltf-viewer hello-ar hello-gltf hello-pbr hello-triangle transparent-rendering"

function build_project {
    local project="$1"
    local configuration="$2"

    if ! [[ "${configuration}" == "Debug" || "${configuration}" == "Release" ]]; then
        echo "Incorrect configuration: ${configuration}"
        echo "Must be either: Debug or Release"
        exit 1
    fi

    xcodebuild \
        -project "${project}/${project}.xcodeproj" \
        -scheme "${project} Metal" \
        -configuration "Metal ${configuration}" \
        build \
        ${XCODEBUILD_FLAGS} \
        HOST_TOOLS_PATH="../../../out/cmake-release/tools"
}

for project in ${PROJECTS}; do
    if [[ ${BUILD_DEBUG} ]]; then
        build_project "${project}" "Debug"
    fi
    if [[ ${BUILD_RELEASE} ]]; then
        build_project "${project}" "Release"
    fi
done
