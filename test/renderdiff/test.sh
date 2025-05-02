# Copyright (C) 2024 The Android Open Source Project
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

OUTPUT_DIR="$(pwd)/out/renderdiff_tests"
RENDERDIFF_TEST_DIR="$(pwd)/test/renderdiff"
TEST_UTILS_DIR="$(pwd)/test/utils"
MESA_DIR="$(pwd)/mesa/out/"
VENV_DIR="$(pwd)/venv"

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
    else
        if [ ! -d ${MESA_LIB_DIR} ]; then
            bash ${TEST_UTILS_DIR}/get_mesa.sh
        fi

        # Install python deps
        python3 -m venv ${VENV_DIR}
        source ${VENV_DIR}/bin/activate

        NEEDED_PYTHON_DEPS=("numpy" "tifffile")
        for cmd in "${NEEDED_PYTHON_DEPS[@]}"; do
            if ! python3 -m pip show -q "${cmd}"; then
                python3 -m pip install ${cmd}
            fi
        done
    fi

}

function end_() {
    if [[ "$GITHUB_WORKFLOW" ]]; then
        set +ex
    else
        deactivate # End python virtual env
    fi
}

# Following steps are taken:
#  - Get and build mesa
#  - Build gltf_viewer
#  - Run the python script that runs the test
#  - Zip up the result

GOLDEN_BRANCH_PARAM='--golden_branch=main'

if [ "$1" == "generate" ]; then
    GOLDEN_BRANCH_PARAM=''
fi

start_ && \
    mkdir -p ${OUTPUT_DIR} && \
    CXX=`which clang++` CC=`which clang` ./build.sh -f -X ${MESA_DIR} -p desktop debug gltf_viewer && \
    python3 ${RENDERDIFF_TEST_DIR}/src/run.py \
            --gltf_viewer="$(pwd)/out/cmake-debug/samples/gltf_viewer" \
            --test=${RENDERDIFF_TEST_DIR}/tests/presubmit.json \
            --output_dir=${OUTPUT_DIR} \
            --opengl_lib=${MESA_LIB_DIR} \
            ${GOLDEN_BRANCH_PARAM} && \
    end_
