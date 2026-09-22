#!/bin/bash

source `dirname $0`/../common/ci-check.sh

set -e
set -x

source `dirname $0`/../common/build-common.sh
pushd `dirname $0`/../.. > /dev/null

# -y none disables the separate prebuilt-tools pass. The wasm path already builds the host tools
# itself, in Release in out/cmake-release, and exports them for the cross build; the extra pass
# only compiles the same ~500 translation units a second time. Unlike the desktop wrappers this
# holds for debug too, which is why split-build.sh is not sourced here.
./build.sh -W -y none -p wasm -c $GENERATE_ARCHIVES $BUILD_DEBUG $BUILD_RELEASE
