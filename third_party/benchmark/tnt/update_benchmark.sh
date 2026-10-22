#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] [--sha <commit_sha> | <version_tag>]"
  echo "  -h: Display this help message"
  echo "  <version_tag>: The benchmark version to download (e.g., 1.8.3)"
  echo "  --sha <commit_sha>: The benchmark commit SHA to download instead of a version tag."
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
    DOWNLOAD_URL="https://github.com/google/benchmark/archive/${DOWNLOAD_REF}.zip"
    ZIP_FILE_NAME="${DOWNLOAD_REF}.zip"
else
    if [[ $# -ne 1 ]]; then
      usage
    fi
    DOWNLOAD_REF=$1
    # The tags for benchmark are like 'v1.8.3'
    DOWNLOAD_URL="https://github.com/google/benchmark/archive/refs/tags/v${DOWNLOAD_REF}.zip"
    ZIP_FILE_NAME="v${DOWNLOAD_REF}.zip"
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

echo "Downloading benchmark ${DOWNLOAD_REF}..."
if ! curl -f -L -o "${ZIP_FILE_NAME}" "${DOWNLOAD_URL}"; then
    echo "Error: Failed to download benchmark ${DOWNLOAD_REF}." >&2
    echo "Please check the version number/SHA and your network connection." >&2
    rm -f "${ZIP_FILE_NAME}"
    exit 1
fi

echo "Unzipping..."

TEMP_DIR="benchmark_temp_unzip"
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

echo "Replacing existing benchmark with new version..."
# If benchmark_new exists, remove it.
rm -rf benchmark_new
mv "${EXTRACTED_DIR}" benchmark_new

rsync -a --delete --exclude=tnt/ benchmark_new/ benchmark/

echo "Cleaning up..."
rm -rf "${ZIP_FILE_NAME}" benchmark_new "${TEMP_DIR}"

echo "Staging changes..."
git add benchmark

echo "Done. Please commit the changes."
