#!/bin/bash

# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e # Fail on any error.

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

set -x # Display commands being run.

cd github/cppdap

git submodule update --init

if [ "$BUILD_SYSTEM" == "cmake" ]; then
    using cmake-3.17.2
    using gcc-9

    mkdir build
    cd build

    build_and_run() {
        cmake .. -DCPPDAP_BUILD_EXAMPLES=1 -DCPPDAP_BUILD_TESTS=1 -DCPPDAP_WARNINGS_AS_ERRORS=1 $1
        make --jobs=$(nproc)

        ./cppdap-unittests
    }

    if [ "$BUILD_SANITIZER" == "asan" ]; then
        build_and_run "-DCPPDAP_ASAN=1"
    elif [ "$BUILD_SANITIZER" == "msan" ]; then
        build_and_run "-DCPPDAP_MSAN=1"
    elif [ "$BUILD_SANITIZER" == "tsan" ]; then
        build_and_run "-DCPPDAP_TSAN=1"
    else
        build_and_run
    fi
else
    echo "Unknown build system: $BUILD_SYSTEM"
    exit 1
fi
