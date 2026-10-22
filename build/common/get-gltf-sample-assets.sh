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
set -e

GLTF_SAMPLE_ASSETS_COMMIT=${GITHUB_GLTF_SAMPLE_ASSETS_COMMIT:-d441dfdb87413ff412c620849a649d61789a470f}
COMMIT_HASH="${GLTF_SAMPLE_ASSETS_COMMIT}"
REPO_URL="https://github.com/KhronosGroup/glTF-Sample-Assets.git"
TARGET_DIR="gltf"

# The default directories to check out if none are specified
DEFAULT_SPARSE_PATHS=(
    "Models/Box/"
    "Models/Triangle/"
    "Models/AnimatedCube/"
)

# Check if command-line arguments are provided
if [ "$#" -gt 0 ]; then
    # If arguments are provided, use them as the paths
    SPARSE_PATHS=()
    for model_name in "$@"; do
        SPARSE_PATHS+=("Models/${model_name}/")
    done
    echo "Downloading specified models: $@"
else
    # Otherwise, use the default list
    SPARSE_PATHS=("${DEFAULT_SPARSE_PATHS[@]}")
    echo "No models specified, downloading default set."
fi

echo "Removing old directory..."
rm -rf "${TARGET_DIR}"

# Clone the repository using a "treeless" clone, which is highly efficient.
# --filter=tree:0: Clones only the repository structure without file content (no historical directory listings), making the initial clone very small.
# --no-checkout:   Prevents automatically checking out the main branch. We will check out a specific commit later.
# --sparse:        Initializes the repository for sparse checkout, allowing us to fetch only specific directories.
git clone --filter=tree:0 --no-checkout --sparse "${REPO_URL}" "${TARGET_DIR}"

cd "${TARGET_DIR}"

git sparse-checkout set "${SPARSE_PATHS[@]}"

echo "Checking out commit ${COMMIT_HASH}..."
git checkout "${COMMIT_HASH}"

echo "Successfully checked out the specified models into the '${TARGET_DIR}' directory."
