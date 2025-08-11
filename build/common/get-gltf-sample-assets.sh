#!/bin/bash
set -e

GLTF_SAMPLE_ASSETS_COMMIT=${GITHUB_GLTF_SAMPLE_ASSETS_COMMIT:-d441dfdb87413ff412c620849a649d61789a470f}
COMMIT_HASH="${GLTF_SAMPLE_ASSETS_COMMIT}"
REPO_URL="https://github.com/KhronosGroup/glTF-Sample-Assets.git"
TARGET_DIR="gltf"

# The directories to check out
SPARSE_PATHS=(
    "Models/Box/"
    "Models/Triangle/"
    "Models/AnimatedCube/"
)

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
