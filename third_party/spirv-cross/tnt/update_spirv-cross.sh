#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] [--sha <commit_sha> | <version_tag>]"
  echo "  -h: Display this help message"
  echo "  <version_tag>: The SPIRV-Cross version to download (e.g., 2023-08-23)"
  echo "  --sha <commit_sha>: The SPIRV-Cross commit SHA to download instead of a version tag."
  exit 1
}

DOWNLOAD_REF=""
DOWNLOAD_URL=""
ZIP_FILE_NAME=""

if [[ $# -eq 0 || "$1" == "-h" ]]; then
  usage
fi

if [[ "$1" == "--sha" ]]; then
    if [[ $# -ne 2 ]]; then
        echo "Error: --sha option requires a commit SHA." >&2
        usage
    fi
    DOWNLOAD_REF=$2
    DOWNLOAD_URL="https://github.com/KhronosGroup/SPIRV-Cross/archive/${DOWNLOAD_REF}.zip"
    ZIP_FILE_NAME="${DOWNLOAD_REF}.zip"
else
    if [[ $# -ne 1 ]]; then
      usage
    fi
    DOWNLOAD_REF=$1
    DOWNLOAD_URL="https://github.com/KhronosGroup/SPIRV-Cross/archive/refs/tags/${DOWNLOAD_REF}.zip"
    ZIP_FILE_NAME="${DOWNLOAD_REF}.zip"
fi


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRD_PARTY_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"

if ! command -v curl &> /dev/null; then
    echo "Error: curl is not installed." >&2
    exit 1
fi

if ! command -v unzip &> /dev/null; then
    echo "Error: unzip is not installed." >&2
    exit 1
fi

if ! command -v patch &> /dev/null; then
    echo "Error: patch is not installed." >&2
    exit 1
fi

cd "${THIRD_PARTY_DIR}"

echo "Downloading SPIRV-Cross ${DOWNLOAD_REF}..."
if ! curl -f -L -o "${ZIP_FILE_NAME}" "${DOWNLOAD_URL}"; then
    echo "Error: Failed to download SPIRV-Cross ${DOWNLOAD_REF}." >&2
    echo "Please check the version number/SHA and your network connection." >&2
    rm -f "${ZIP_FILE_NAME}"
    exit 1
fi

echo "Unzipping..."

TEMP_DIR="spirv-cross_temp_unzip"
# Clean up temp dir from previous runs, if any.
rm -rf "${TEMP_DIR}"
mkdir "${TEMP_DIR}"

if ! unzip -q "${ZIP_FILE_NAME}" -d "${TEMP_DIR}"; then
    echo "Error: Failed to unzip the downloaded file." >&2
    rm -f "${ZIP_FILE_NAME}"
    rm -rf "${TEMP_DIR}"
    exit 1
fi

# The archive contains a single directory. Find it.
EXTRACTED_DIR=$(find "${TEMP_DIR}" -mindepth 1 -maxdepth 1 -type d)

if [ -z "${EXTRACTED_DIR}" ] || [ ! -d "${EXTRACTED_DIR}" ]; then
    echo "Error: Could not find the extracted directory inside the archive." >&2
    rm -f "${ZIP_FILE_NAME}"
    rm -rf "${TEMP_DIR}"
    exit 1
fi

echo "Replacing existing spirv-cross with new version..."
# If spirv-cross_new exists, remove it.
rm -rf spirv-cross_new
mv "${EXTRACTED_DIR}" spirv-cross_new

rsync -a --delete --exclude=tnt/ spirv-cross_new/ spirv-cross/

echo "Applying patches..."
if ! patch -p2 < spirv-cross/tnt/0001-convert-floats-to-their-smallest-string-representati.patch; then
    echo "Error: Failed to apply 0001-convert-floats-to-their-smallest-string-representati.patch" >&2
    exit 1
fi

if ! patch -p2 < spirv-cross/tnt/0002-localeconv-api-level-check.patch; then
    echo "Error: Failed to apply 0002-localeconv-api-level-check.patch" >&2
    exit 1
fi

echo "Cleaning up..."
rm -rf "${ZIP_FILE_NAME}" spirv-cross_new "${TEMP_DIR}"

echo "Staging changes..."
git add spirv-cross

echo "Done. Please commit the changes."
