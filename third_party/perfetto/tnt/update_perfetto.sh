#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] <version_tag>"
  echo "  -h: Display this help message"
  echo "  <version_tag>: The Perfetto version to download (e.g., v50.1)"
  exit 1
}

if [[ $# -ne 1 || "$1" == "-h" ]]; then
  usage
fi

DOWNLOAD_REF=$1
DOWNLOAD_URL="https://github.com/google/perfetto/archive/refs/tags/${DOWNLOAD_REF}.zip"
ZIP_FILE_NAME="${DOWNLOAD_REF}.zip"

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

echo "Downloading Perfetto ${DOWNLOAD_REF}..."
if ! curl -f -L -o "${ZIP_FILE_NAME}" "${DOWNLOAD_URL}"; then
    echo "Error: Failed to download Perfetto ${DOWNLOAD_REF}." >&2
    echo "Please check the version number and your network connection." >&2
    rm -f "${ZIP_FILE_NAME}"
    exit 1
fi

echo "Unzipping..."

TEMP_DIR="perfetto_temp_unzip"
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

echo "Replacing existing perfetto contents with new version..."
# If perfetto_new exists, remove it.
rm -rf perfetto_new
mv "${EXTRACTED_DIR}" perfetto_new

# The perfetto directory should contain the content of the sdk directory from the archive, and our tnt directory.
rsync -a --delete perfetto_new/sdk/ perfetto/perfetto/

echo "Cleaning up..."
rm -rf "${ZIP_FILE_NAME}" perfetto_new "${TEMP_DIR}"

echo "Staging changes..."
git add perfetto

echo "Done. Please commit the changes."
