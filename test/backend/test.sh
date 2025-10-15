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

# Build backend_test_mac
echo "Building backend_test_mac..."
"${PROJECT_ROOT_DIR}/build.sh" -W -p desktop -X "${PROJECT_ROOT_DIR}/mesa" debug backend_test_mac

# Set environment variables to use Mesa drivers.
export DYLD_LIBRARY_PATH="${PROJECT_ROOT_DIR}/mesa/out/lib"
export VK_ICD_FILENAMES="${PROJECT_ROOT_DIR}/mesa/out/share/vulkan/icd.d/lvp_icd.aarch64.json"

# Run the backend test.
${PROJECT_ROOT_DIR}/out/cmake-debug/filament/backend/backend_test_mac -a opengl --headless_only
