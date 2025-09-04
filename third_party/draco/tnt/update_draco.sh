#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] <version_tag>"
  echo "  -h: Display this help message"
  echo "  <version_tag>: The draco version to download (e.g., 1.4.1)"
  exit 1
}

if [[ $# -ne 1 || "$1" == "-h" ]]; then
  usage
fi

VERSION=$1
DOWNLOAD_URL="https://github.com/google/draco/archive/${VERSION}.zip"
ZIP_FILE_NAME="draco-${VERSION}.zip"

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

if ! command -v rsync &> /dev/null; then
    echo "Error: rsync is not installed." >&2
    exit 1
fi

cd "${THIRD_PARTY_DIR}"

echo "Downloading draco ${VERSION}..."
if ! curl -f -L -o "${ZIP_FILE_NAME}" "${DOWNLOAD_URL}"; then
    echo "Error: Failed to download draco ${VERSION}." >&2
    echo "Please check the version number and your network connection." >&2
    rm -f "${ZIP_FILE_NAME}"
    exit 1
fi

echo "Unzipping..."

TEMP_DIR="draco_temp_unzip"
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

echo "Replacing existing draco with new version..."
# If draco_new exists, remove it.
rm -rf draco_new
mv "${EXTRACTED_DIR}" draco_new

rsync -a --delete --exclude=tnt/ --exclude=testdata/ --exclude=unity/ --exclude=maya/ draco_new/ draco/

echo "Cleaning up..."
rm -rf "${ZIP_FILE_NAME}" draco_new "${TEMP_DIR}"

echo "Staging changes..."
git add draco

echo "Done. Please commit the changes."
