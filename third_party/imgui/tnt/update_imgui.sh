#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] <version>"
  echo "  -h: Display this help message"
  echo "  <version>: The imgui version to download (e.g., 1.90.9)"
  exit 1
}

if [[ $# -eq 0 || "$1" == "-h" ]]; then
  usage
fi

IMGUI_VERSION=$1
ZIP_FILE_NAME="v${IMGUI_VERSION}.zip"

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

echo "Downloading imgui version ${IMGUI_VERSION}..."
if ! curl -f -L -o "${ZIP_FILE_NAME}" "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip"; then
    echo "Error: Failed to download imgui version ${IMGUI_VERSION}." >&2
    echo "Please check the version number and your network connection." >&2
    rm -f "${ZIP_FILE_NAME}"
    exit 1
fi

echo "Unzipping..."
TEMP_DIR="imgui_temp_unzip"
rm -rf "${TEMP_DIR}"
mkdir "${TEMP_DIR}"

if ! unzip -q "${ZIP_FILE_NAME}" -d "${TEMP_DIR}"; then
    echo "Error: Failed to unzip the downloaded file." >&2
    rm -f "${ZIP_FILE_NAME}"
    rm -rf "${TEMP_DIR}"
    exit 1
fi

EXTRACTED_DIR=$(find "${TEMP_DIR}" -mindepth 1 -maxdepth 1 -type d)

if [ -z "${EXTRACTED_DIR}" ] || [ ! -d "${EXTRACTED_DIR}" ]; then
    echo "Error: Could not find the extracted directory inside the archive." >&2
    rm -f "${ZIP_FILE_NAME}"
    rm -rf "${TEMP_DIR}"
    exit 1
fi

echo "Replacing existing imgui with new version..."
rm -rf imgui_new
mv "${EXTRACTED_DIR}" imgui_new
rsync -a --delete --exclude=tnt/ imgui_new/ imgui/

echo "Removing unnecessary files..."
rm -f imgui/examples/libs/glfw/lib-vc2010-32/*.lib
rm -f imgui/examples/libs/glfw/lib-vc2010-64/*.lib

echo "Cleaning up..."
rm -f "${ZIP_FILE_NAME}"
rm -rf imgui_new "${TEMP_DIR}"

echo "Staging changes..."
git add imgui

echo "Done. Please commit the changes."
