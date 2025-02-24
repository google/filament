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
set -x # Display commands being run.

BUILD_ROOT=$PWD

cd github/cppdap

git submodule update --init

if [ "$BUILD_SYSTEM" == "cmake" ]; then
    mkdir build
    cd build

    cmake .. -DCPPDAP_BUILD_EXAMPLES=1 -DCPPDAP_BUILD_TESTS=1 -DCPPDAP_WARNINGS_AS_ERRORS=1
    make -j$(sysctl -n hw.logicalcpu)

    ./cppdap-unittests
else
    echo "Unknown build system: $BUILD_SYSTEM"
    exit 1
fi
