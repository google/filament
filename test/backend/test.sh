#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Get the directory of this script.
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PROJECT_ROOT_DIR="${SCRIPT_DIR}/../.."

# Build Mesa if it's not already built.
if [ ! -d "${PROJECT_ROOT_DIR}/mesa/out" ]; then
    echo "Mesa not found. Building Mesa..."
    "${PROJECT_ROOT_DIR}/build/common/get-mesa.sh"
fi

BACKEND_TEST_TARGET=''

# Set environment variables to use Mesa drivers.
os_name=$(uname -s)
if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH="${PROJECT_ROOT_DIR}/mesa/out/lib/x86_64-linux-gnu"
    export VK_ICD_FILENAMES="${PROJECT_ROOT_DIR}/mesa/out/share/vulkan/icd.d/lvp_icd.x86_64.json"
    BACKEND_TEST_TARGET=backend_test_linux
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_LIBRARY_PATH="${PROJECT_ROOT_DIR}/mesa/out/lib"
    export VK_ICD_FILENAMES="${PROJECT_ROOT_DIR}/mesa/out/share/vulkan/icd.d/lvp_icd.aarch64.json"
    BACKEND_TEST_TARGET=backend_test_mac
fi

# Build backend_test_mac
echo "Building backend_test_mac..."
"${PROJECT_ROOT_DIR}/build.sh" -W -p desktop -X "${PROJECT_ROOT_DIR}/mesa" debug ${BACKEND_TEST_TARGET}

set +e

GTEST_FILTER_ARG=""
BACKENDS=("opengl" "vulkan")
for arg in "$@"
do
    if [[ "$arg" == --gtest_filter* ]] ; then
        GTEST_FILTER_ARG="$arg"
    fi
    if [[ "$arg" == --backend* ]] ; then
        BACKENDS=("${arg#*=}")
    fi
done

FINAL_RESULT=0
for BACKEND in ${BACKENDS[@]}; do
    echo "----- ${BACKEND} backend test -----"
    ${PROJECT_ROOT_DIR}/out/cmake-debug/filament/backend/${BACKEND_TEST_TARGET} -a ${BACKEND} --ci --headless_only ${GTEST_FILTER_ARG}
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
