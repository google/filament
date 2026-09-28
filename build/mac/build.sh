#!/bin/bash

source `dirname $0`/../common/ci-check.sh

set -e
set -x

source `dirname $0`/../common/build-common.sh
# Must be sourced before the pushd below: $0 is fixed at process start, so `dirname $0` is only
# meaningful while the working directory is still the one the script was invoked from.
source `dirname $0`/../common/split-build.sh

pushd `dirname $0`/../.. > /dev/null

./build.sh -c $SPLIT_BUILD_OPTION $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
