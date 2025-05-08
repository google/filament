#!/bin/bash

source `dirname $0`/../common/ci-check.sh

set -e
set -x

source `dirname $0`/../common/build-common.sh
pushd `dirname $0`/../.. > /dev/null

./build.sh -c $RUN_TESTS $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
