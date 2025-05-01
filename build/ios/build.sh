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

source `dirname $0`/../common/ci-common.sh
source `dirname $0`/../common/build-common.sh

pushd `dirname $0`/../.. > /dev/null

# If we're generating an archive for release or continuous builds, then we'll also build for the
# simulator and create universal libraries.
BUILD_SIMULATOR=
if [[ "${GENERATE_ARCHIVES}" ]]; then
    BUILD_SIMULATOR="-s -l"
fi

./build.sh -i -p ios -c $BUILD_SIMULATOR $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
