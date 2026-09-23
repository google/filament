#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Get the directory of this script.
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_ROOT_DIR="${SCRIPT_DIR}/../.."

# These two arguments have to be known before the build, because they decide what gets built and
# with which flags. The rest are parsed after it.
BUILD_ONLY=false
REQUESTED_BACKEND=''
for arg in "$@"
do
    if [[ "$arg" == "--build-only" ]] ; then
        BUILD_ONLY=true
    fi
    if [[ "$arg" == --backend* ]] ; then
        REQUESTED_BACKEND="${arg#*=}"
    fi
done

os_name=$(uname -s)
arch_name=$(uname -m)

# Metal talks to the platform's own driver so no need for software rasterization setup.
METAL_ONLY=false
if [[ "$os_name" == "Darwin" && "${REQUESTED_BACKEND}" == "metal" ]]; then
    METAL_ONLY=true
fi

# Build Mesa if it's not already built.
if [[ "${METAL_ONLY}" == "false" && ! -d "${PROJECT_ROOT_DIR}/mesa/out" ]]; then
    echo "Mesa not found. Building Mesa..."
    "${PROJECT_ROOT_DIR}/build/common/get-mesa.sh"
fi

BACKEND_TEST_TARGET=''
ASAN_FLAG=''

# Set environment variables to use Mesa drivers.
if [[ "$os_name" == "Linux" ]]; then
    if [[ "$arch_name" == "aarch64" ]]; then
        export LD_LIBRARY_PATH="${PROJECT_ROOT_DIR}/mesa/out/lib/aarch64-linux-gnu"
        export VK_ICD_FILENAMES="${PROJECT_ROOT_DIR}/mesa/out/share/vulkan/icd.d/lvp_icd.aarch64.json"
    else
        export LD_LIBRARY_PATH="${PROJECT_ROOT_DIR}/mesa/out/lib/x86_64-linux-gnu"
        export VK_ICD_FILENAMES="${PROJECT_ROOT_DIR}/mesa/out/share/vulkan/icd.d/lvp_icd.x86_64.json"
    fi
    BACKEND_TEST_TARGET=backend_test_linux
    ASAN_FLAG="-b"
elif [[ "$os_name" == "Darwin" ]]; then
    if [[ "${METAL_ONLY}" == "false" ]]; then
        export DYLD_LIBRARY_PATH="${PROJECT_ROOT_DIR}/mesa/out/lib"
        export VK_ICD_FILENAMES="${PROJECT_ROOT_DIR}/mesa/out/share/vulkan/icd.d/lvp_icd.aarch64.json"
    fi
    BACKEND_TEST_TARGET=backend_test_mac
    # asan is too slow for macOs build of the backend test
    ASAN_FLAG=""
fi

# Flags that vary by configuration. The defaults describe the OSMesa-backed debug build that the
# opengl, vulkan and webgpu runs need on both hosts.
BUILD_FLAGS=(-W -y release -X "${PROJECT_ROOT_DIR}/mesa")
BUILD_TYPE=debug
BUILD_DIR=cmake-debug

if [[ "${METAL_ONLY}" == "true" ]]; then
    # Note that these flags should match the ccache build flags in the postsubmit; so that we'd hit
    # the cache.
    BUILD_FLAGS=(-y release)
    BUILD_TYPE=debug
    BUILD_DIR=cmake-debug
fi

# Build backend test
echo "Building ${BACKEND_TEST_TARGET}..."
"${PROJECT_ROOT_DIR}/build.sh" ${ASAN_FLAG} "${BUILD_FLAGS[@]}" -p desktop "${BUILD_TYPE}" ${BACKEND_TEST_TARGET}

# Used by the ccache warming job, which wants the compiler cache this build populates but has no
# reason to run the tests themselves. Going through this script rather than repeating the build
# line above is what keeps the cached objects flag-identical to what a real run asks for.
if [[ "${BUILD_ONLY}" == "true" ]]; then
    echo "--build-only given; skipping the test run."
    exit 0
fi

set +e

GTEST_FILTER_ARG=""
BACKENDS=("opengl" "vulkan" "webgpu")
if [[ -n "${REQUESTED_BACKEND}" ]]; then
    BACKENDS=("${REQUESTED_BACKEND}")
fi
for arg in "$@"
do
    if [[ "$arg" == --gtest_filter* ]] ; then
        GTEST_FILTER_ARG="$arg"
    fi
done

# Mesa OSMesa is known to leak part of the context.
LSAN_CMD_PREFIX=""
if [[ "${ASAN_FLAG}" == "-b" ]]; then
    echo "leak:PlatformOSMesa.cpp" > leak_skip.txt
    export LSAN_OPTIONS=suppressions=leak_skip.txt
fi

FINAL_RESULT=0
for BACKEND in ${BACKENDS[@]}; do
    echo "----- ${BACKEND} backend test -----"

    ${PROJECT_ROOT_DIR}/out/${BUILD_DIR}/filament/backend/${BACKEND_TEST_TARGET} \
                           -a ${BACKEND} --ci --headless_only ${GTEST_FILTER_ARG}

    RESULT=$(echo $?)
    if [ ${RESULT} -gt 0 ]; then
        echo "----- Error: backend ${BACKEND} test failed with result ${RESULT} -----"
        FINAL_RESULT=${RESULT}
    fi
done

if [ ${FINAL_RESULT} -gt 0 ]; then
    exit 1
fi
exit 0
