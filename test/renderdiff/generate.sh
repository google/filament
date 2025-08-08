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

source `dirname $0`/src/preamble.sh

function start_render_() {
    start_
    if [[ ! "$GITHUB_WORKFLOW" ]]; then
        if [ ! -d ${MESA_LIB_DIR} ]; then
            bash ${BUILD_COMMON_DIR}/get-mesa.sh
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
    # -W enables the webgpu build
    # -f forces regeneration of cmake build files
    # -X points to the mesa directory, which contains the compiled gl and vk drivers.
    CXX=`which clang++` CC=`which clang` ./build.sh -W -f -X ${MESA_DIR} -p desktop debug gltf_viewer
}

function end_render_() {
    if [[ ! "$GITHUB_WORKFLOW" ]]; then
        deactivate # End python virtual env
    fi
    end_
}

# Following steps are taken:
#  - Get and build mesa
#  - Build gltf_viewer
#  - Run a test

start_render_ && \
    python3 ${RENDERDIFF_TEST_DIR}/src/render.py \
            --gltf_viewer="$(pwd)/out/cmake-debug/samples/gltf_viewer" \
            --test=${RENDERDIFF_TEST_DIR}/tests/presubmit.json \
            --output_dir=${RENDER_OUTPUT_DIR} \
            --opengl_lib=${MESA_LIB_DIR} && \
    end_render_
