#!/bin/bash

source `dirname $0`/../common/ci-check.sh

set -e
set -x

source `dirname $0`/../common/build-common.sh
pushd `dirname $0`/../.. > /dev/null

# If we're generating an archive for release or continuous builds, then we'll also build for the
# simulator and create universal libraries.
BUILD_SIMULATOR=
if [[ "${GENERATE_ARCHIVES}" ]]; then
    BUILD_SIMULATOR="-s -l"
fi

./build.sh -i -p ios -c $BUILD_SIMULATOR $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
