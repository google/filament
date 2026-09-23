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

        if [ ! -d ${GLTF_DIR} ]; then
            cat ${RENDERDIFF_TEST_DIR}/tests/gltf_models.txt | xargs bash ${BUILD_COMMON_DIR}/get-gltf-sample-assets.sh
        fi

        # Install python deps
        python3 -m venv ${VENV_DIR}
        source ${VENV_DIR}/bin/activate

        NEEDED_PYTHON_DEPS=()
        for cmd in "${NEEDED_PYTHON_DEPS[@]}"; do
            if ! python3 -m pip show -q "${cmd}"; then
                python3 -m pip install ${cmd}
            fi
        done
    fi
    # -W enables the webgpu build
    # -f forces regeneration of cmake build files
    # -X points to the mesa directory, which contains the compiled gl and vk drivers.
    #
    # diffimg is built here rather than from a separate release tree. It is the consumer's own
    # command line that the ccache warming job reproduces through --build-only, so everything the
    # test needs has to be compiled by this one invocation.
    GLTF_VIEWER_PATH="$(pwd)/out/cmake-debug/samples/gltf_viewer"
    HELLOTRIANGLE_PATH="$(pwd)/out/cmake-debug/samples/hellotriangle"
    if [[ "$NOREBUILD" == "true" ]] && [[ -f ${GLTF_VIEWER_PATH} ]] && [[ -f ${HELLOTRIANGLE_PATH} ]] \
       && [[ -f ${DIFFIMG_PATH} ]]; then
        echo "Skipping build of gltf_viewer, filament-samples and diffimg"
    else
        # macOS only: get-mesa leaves Homebrew's clang on PATH, and the samples have to be built
        # with Apple's. On Linux the compiler is deliberately left to CMake, which resolves cc and
        # c++ to the clang the CI prerequisites pin; forcing `which clang` here would instead pick
        # up whichever unversioned clang the runner image happens to ship, and would produce
        # objects no other job's compiler cache agrees with.
        if [[ "$(uname -s)" == "Darwin" ]]; then
            export CXX=`which clang++`
            export CC=`which clang`
        fi
        ./build.sh -f -W -X ${MESA_DIR} -p desktop debug gltf_viewer filament-samples diffimg
    fi
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

TEST_CONFIG="${RENDERDIFF_TEST_DIR}/tests/presubmit.json"

for i in "$@"
do
case $i in
    --test=*)
    TEST_CONFIG="${i#*=}"
    shift # past argument=value
    ;;
    --test_filter=*)
    TEST_FILTER="${i#*=}"
    shift # past argument=value
    ;;
    --no_rebuild)
    NOREBUILD="true"
    shift # past argument with no value
    ;;
    --build-only)
    BUILD_ONLY="true"
    shift # past argument with no value
    ;;
    --num_threads=*)
    NUM_THREADS="${i#*=}"
    shift # past argument=value
    ;;
    *)
          # unknown option
    ;;
esac
done


start_render_ || exit 1

# Used by the ccache warming job, which wants the objects this build produces but has no reason to
# render: presubmit does that. Going through this script rather than repeating the build line above
# is what keeps the cached objects flag-identical to what a real run asks for.
if [[ "$BUILD_ONLY" == "true" ]]; then
    echo "--build-only given; skipping the render."
    end_render_
    exit 0
fi

for backend in opengl vulkan webgpu; do
    FILAMENT_VK_ICD="${MESA_VK_ICD_PATH}" FILAMENT_OPENGL_LIB="${MESA_LIB_DIR}" \
    python3 ${RENDERDIFF_TEST_DIR}/src/render.py \
            --executable="$(pwd)/out/cmake-debug/samples/gltf_viewer" \
            --platform=desktop \
            --backend=$backend \
            --test="${TEST_CONFIG}" \
            --output_dir="${RENDER_OUTPUT_DIR}" \
            ${TEST_FILTER:+--test_filter="$TEST_FILTER"} \
            ${NUM_THREADS:+--num_threads="$NUM_THREADS"} || exit 1
done

end_render_
