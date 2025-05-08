# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/usr/bin/bash

# Sets up the environment for scripts in test/renderdiff/

OUTPUT_DIR="$(pwd)/out/renderdiff_tests"
RENDERDIFF_TEST_DIR="$(pwd)/test/renderdiff"
MESA_DIR="$(pwd)/mesa/out/"
VENV_DIR="$(pwd)/venv"
BUILD_COMMON_DIR="$(pwd)/build/common"

os_name=$(uname -s)
if [[ "$os_name" == "Linux" ]]; then
    MESA_LIB_DIR="${MESA_DIR}lib/x86_64-linux-gnu"
elif [[ "$os_name" == "Darwin" ]]; then
    MESA_LIB_DIR="${MESA_DIR}lib"
else
    echo "Unsupported platform for renderdiff tests"
    exit 1
fi

function start_() {
    if [[ "$GITHUB_WORKFLOW" ]]; then
        set -ex
    fi
}

function end_() {
    if [[ "$GITHUB_WORKFLOW" ]]; then
        set +ex
    fi
}
