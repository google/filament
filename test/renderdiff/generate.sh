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

# Which platform's renderers to run. 'desktop' builds and runs the native gltf_viewer;
# 'web' builds the wasm gltf_viewer and drives it through a headless browser. They have
# almost disjoint prerequisites (mesa/vulkan vs. emsdk/playwright) and in CI they run on
# separate matrix legs, so only the requested one is set up.
PLATFORM="desktop"

DESKTOP_BACKENDS="opengl vulkan webgpu"
WEB_BACKENDS="webgl webgpu"

WASM_SAMPLES_DIR="$(pwd)/out/cmake-wasm-debug/samples"

function common_setup_() {
    start_
    if [[ ! "$GITHUB_WORKFLOW" ]]; then
        if [ ! -d ${GLTF_DIR} ]; then
            cat ${RENDERDIFF_TEST_DIR}/tests/gltf_models.txt | xargs bash ${BUILD_COMMON_DIR}/get-gltf-sample-assets.sh
        fi

        # Install python deps
        python3 -m venv ${VENV_DIR}
        source ${VENV_DIR}/bin/activate
    fi
}

function setup_desktop_() {
    if [[ ! "$GITHUB_WORKFLOW" ]]; then
        if [ ! -d ${MESA_LIB_DIR} ]; then
            bash ${BUILD_COMMON_DIR}/get-mesa.sh
        fi
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
        ./build.sh -f -W -X ${MESA_DIR} -p desktop debug gltf_viewer filament-samples diffimg || return 1
    fi
}

function setup_web_() {
    if [[ ! "${EMSDK}" ]]; then
        echo "EMSDK is not set; the wasm build needs an emscripten SDK." \
             "In CI this is done by .github/actions/web-prereq; locally run" \
             "build/common/get-emscripten.sh and export EMSDK." >&2
        return 1
    fi

    # The flags mirror build/web/build.sh, which is what the postsubmit build-web job runs to fill
    # the web-emsdk compiler cache that the web renderdiff jobs restore. Both builds configure the
    # same trees with the same options, so their compile lines match:
    #   -W       enables the webgpu backend, which the web-webgpu renderer needs, and makes the
    #            wasm build use the in-tree emdawnwebgpu port.
    #   -y none  skips the separate prebuilt-tools pass. The wasm path already builds the host
    #            tools in Release in out/cmake-release; the extra pass would configure a tree the
    #            cache holds nothing for.
    # build-web does not compile gltf_viewer, which is excluded from the default wasm build, so it
    # also runs this script with --build-only to put these objects in the same cache.
    if [[ "$NOREBUILD" == "true" ]] && [[ -f "${WASM_SAMPLES_DIR}/gltf_viewer.wasm" ]]; then
        echo "Skipping wasm build of gltf_viewer"
    else
        ./build.sh -W -y none -p wasm debug gltf_viewer || return 1
    fi

    # diffimg compares the renders against the goldens, so it is a host tool regardless of the
    # platform being rendered. It goes into the release tree the wasm build just configured for
    # its host tools, with the same flags, so this only compiles diffimg's own sources.
    if [[ "$NOREBUILD" == "true" ]] && [[ -f ${WEB_DIFFIMG_PATH} ]]; then
        echo "Skipping build of diffimg"
    else
        ./build.sh -W -y none -p desktop release diffimg || return 1
    fi
}

# Installs the browser the web renderer drives. Separate from setup_web_ because it is only needed
# to render: the ccache warmer runs with --build-only and has no use for a browser.
function prepare_web_browser_() {
    # The web renderer drives a real browser through playwright. The exact browser build is
    # part of the golden contract (see rendering_requirements.txt), so install the version
    # playwright pins rather than whatever the runner happens to have.
    python3 -m pip install -r ${RENDERDIFF_TEST_DIR}/src/rendering_requirements.txt || return 1
    python3 -m playwright install --with-deps chromium || return 1
}

function end_render_() {
    if [[ ! "$GITHUB_WORKFLOW" ]]; then
        deactivate # End python virtual env
    fi
    end_
}

# Following steps are taken:
#  - Get the prerequisites for the requested platform (mesa, or emsdk + a browser)
#  - Build gltf_viewer for that platform
#  - Run a test against each of that platform's backends

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
    --platform=*)
    PLATFORM="${i#*=}"
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

case "${PLATFORM}" in
    desktop) BACKENDS="${DESKTOP_BACKENDS}" ;;
    web)     BACKENDS="${WEB_BACKENDS}" ;;
    *)
        echo "Unknown --platform=${PLATFORM}; expected 'desktop' or 'web'." >&2
        exit 1
        ;;
esac

common_setup_ || exit 1
setup_${PLATFORM}_ || exit 1

# Used by the ccache warming job, which wants the objects this build produces but has no reason to
# render: presubmit does that. Going through this script rather than repeating the build line above
# is what keeps the cached objects flag-identical to what a real run asks for.
if [[ "$BUILD_ONLY" == "true" ]]; then
    echo "--build-only given; skipping the render."
    end_render_
    exit 0
fi

if [[ "${PLATFORM}" == "web" ]]; then
    prepare_web_browser_ || exit 1
fi

# Let a crashing render leave a core behind. This costs nothing unless something dies on a signal.
# Where the core lands is a root-only system setting, which the CI sets separately;
# report_crashes.sh knows how to find cores either way.
ulimit -c unlimited 2> /dev/null || \
    echo "Could not raise the core dump limit; a crashing render will not leave a backtrace."

# A trap rather than a statement after the loop, because the loop is not the only way out of this
# script: `set -e` is on under CI, and an interrupted run is exactly the kind that has left a core
# behind.
touch "${RENDER_START_MARKER}"
trap 'bash ${RENDERDIFF_TEST_DIR}/src/report_crashes.sh' EXIT

# Every backend renders, whatever the ones before it did, and the failure is reported once the loop
# is done. Stopping at the first failing backend left the later ones unrendered, which the
# comparison downstream cannot tell apart from a render that produced no image: one crash reported
# every golden of the two remaining backends as missing, burying the test that actually broke.
render_status=0
for backend in ${BACKENDS}; do
    if [[ "${PLATFORM}" == "web" ]]; then
        # --executable is the emscripten bundle directory, not a binary: the renderer needs
        # the shell, the .js loader and the .wasm together. Rendering is sequential (one page
        # per test), so --num_threads does not apply.
        python3 ${RENDERDIFF_TEST_DIR}/src/render.py \
                --executable="${WASM_SAMPLES_DIR}" \
                --platform=web \
                --backend=$backend \
                --test="${TEST_CONFIG}" \
                --output_dir="${RENDER_OUTPUT_DIR}" \
                ${TEST_FILTER:+--test_filter="$TEST_FILTER"} || render_status=1
    else
        FILAMENT_VK_ICD="${MESA_VK_ICD_PATH}" FILAMENT_OPENGL_LIB="${MESA_LIB_DIR}" \
        python3 ${RENDERDIFF_TEST_DIR}/src/render.py \
                --executable="$(pwd)/out/cmake-debug/samples/gltf_viewer" \
                --platform=desktop \
                --backend=$backend \
                --test="${TEST_CONFIG}" \
                --output_dir="${RENDER_OUTPUT_DIR}" \
                ${TEST_FILTER:+--test_filter="$TEST_FILTER"} \
                ${NUM_THREADS:+--num_threads="$NUM_THREADS"} || render_status=1
    fi
done

end_render_

exit ${render_status}
