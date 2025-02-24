#!/bin/bash

# Copyright 2022 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e # Fail on any error.

function show_cmds { set -x; }
function hide_cmds { { set +x; } 2>/dev/null; }
function task_begin {
    TASK_NAME="$@"
    SECONDS=0
}
function print_last_task_duration {
    if [ ! -z "${TASK_NAME}" ]; then
        echo "${TASK_NAME} completed in $(($SECONDS / 3600))h$((($SECONDS / 60) % 60))m$(($SECONDS % 60))s"
    fi
}
function status {
    echo ""
    echo ""
    print_last_task_duration
    echo ""
    echo "*****************************************************************"
    echo "* $@"
    echo "*****************************************************************"
    echo ""
    task_begin $@
}

CLONE_SRC_DIR="$(pwd)"

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

using cmake-3.31.2
using gcc-13

status "Cloning to clean source directory at '${SRC_DIR}'"

mkdir -p ${SRC_DIR}
cd ${SRC_DIR}
git clone ${CLONE_SRC_DIR} .

mkdir -p build && cd build

if [[ -z "${REACTOR_BACKEND}" ]]; then
  REACTOR_BACKEND="LLVM"
fi

# Lower the amount of debug info, to reduce Kokoro build times.
SWIFTSHADER_LESS_DEBUG_INFO=1

status "Generating CMake build files"
cmake .. \
    "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" \
    "-DREACTOR_BACKEND=${REACTOR_BACKEND}" \
    "-DSWIFTSHADER_LLVM_VERSION=${LLVM_VERSION}" \
    "-DREACTOR_VERIFY_LLVM_IR=1" \
    "-DSWIFTSHADER_LESS_DEBUG_INFO=${SWIFTSHADER_LESS_DEBUG_INFO}" \
    "-DSWIFTSHADER_BUILD_BENCHMARKS=1"

status "Building"
cmake --build . -- -j $(nproc)

status "Running unit tests"

cd .. # Some tests must be run from project root

build/ReactorUnitTests
build/system-unittests
build/vk-unittests

status "Building and running rr::Print unit tests"

cd build
cmake .. "-DREACTOR_ENABLE_PRINT=1"
cmake --build . --target ReactorUnitTests -- -j $(nproc)
cmake .. "-DREACTOR_ENABLE_PRINT=0"
cd ..
build/ReactorUnitTests --gtest_filter=ReactorUnitTests.Print*

status "Building and testing with REACTOR_EMIT_ASM_FILE"

cd build
cmake .. "-DREACTOR_EMIT_ASM_FILE=1"
cmake --build . --target ReactorUnitTests -- -j $(nproc)
cmake .. "-DREACTOR_EMIT_ASM_FILE=0"
cd ..
build/ReactorUnitTests --gtest_filter=ReactorUnitTests.EmitAsm

#status "Building with REACTOR_EMIT_DEBUG_INFO"
#
#cd build
#cmake .. "-DREACTOR_EMIT_DEBUG_INFO=1"
#cmake --build . --target ReactorUnitTests -- -j $(nproc)
#cmake .. "-DREACTOR_EMIT_DEBUG_INFO=0"
#cd ..

#status "Building with REACTOR_EMIT_PRINT_LOCATION"
#
#cd build
#cmake .. "-DREACTOR_EMIT_PRINT_LOCATION=1"
#cmake --build . --target ReactorUnitTests -- -j $(nproc)
#cmake .. "-DREACTOR_EMIT_PRINT_LOCATION=0"
#cd ..

status "Done"
