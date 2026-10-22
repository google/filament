#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] [--sha <commit_sha> | <version_tag>]"
  echo "  -h: Display this help message"
  echo "  <version_tag>: The SPIRV-Headers version to download"
  echo "  --sha <commit_sha>: The SPIRV-Headers commit SHA to download instead of a version tag."
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
    DOWNLOAD_URL="https://github.com/KhronosGroup/SPIRV-Headers/archive/${DOWNLOAD_REF}.zip"
    ZIP_FILE_NAME="${DOWNLOAD_REF}.zip"
else
    if [[ $# -ne 1 ]]; then
      usage
    fi
    DOWNLOAD_REF=$1
    # The tags for SPIRV-Headers are like 'sdk-1.3.268.0'
    DOWNLOAD_URL="https://github.com/KhronosGroup/SPIRV-Headers/archive/refs/tags/sdk-${DOWNLOAD_REF}.zip"
    ZIP_FILE_NAME="sdk-${DOWNLOAD_REF}.zip"
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

cd "${THIRD_PARTY_DIR}"

echo "Downloading SPIRV-Headers ${DOWNLOAD_REF}..."
if ! curl -f -L -o "${ZIP_FILE_NAME}" "${DOWNLOAD_URL}"; then
    echo "Error: Failed to download SPIRV-Headers ${DOWNLOAD_REF}." >&2
    echo "Please check the version number/SHA and your network connection." >&2
    rm -f "${ZIP_FILE_NAME}"
    exit 1
fi

echo "Unzipping..."

TEMP_DIR="spirv-headers_temp_unzip"
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

echo "Replacing existing spirv-headers with new version..."
# If spirv-headers_new exists, remove it.
rm -rf spirv-headers_new
mv "${EXTRACTED_DIR}" spirv-headers_new

rsync -a --delete --exclude=tnt/ spirv-headers_new/ spirv-headers/

echo "Cleaning up..."
rm -rf "${ZIP_FILE_NAME}" spirv-headers_new "${TEMP_DIR}"

echo "Staging changes..."
git add spirv-headers

echo "Done. Please commit the changes."
