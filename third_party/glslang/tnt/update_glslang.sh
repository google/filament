#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] <version>"
  echo "  -h: Display this help message"
  echo "  <version>: The glslang version to download (e.g., 15.4.0)"
  exit 1
}

if [[ $# -eq 0 || "$1" == "-h" ]]; then
  usage
fi

GLSLANG_VERSION=$1

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

echo "Downloading glslang version ${GLSLANG_VERSION}..."
if ! curl -f -L -O "https://github.com/KhronosGroup/glslang/archive/refs/tags/${GLSLANG_VERSION}.zip"; then
    echo "Error: Failed to download glslang version ${GLSLANG_VERSION}." >&2
    echo "Please check the version number and your network connection." >&2
    rm -f "${GLSLANG_VERSION}.zip"
    exit 1
fi

echo "Unzipping..."
if ! unzip -q "${GLSLANG_VERSION}.zip"; then
    echo "Error: Failed to unzip the downloaded file." >&2
    rm -f "${GLSLANG_VERSION}.zip"
    exit 1
fi

echo "Replacing existing glslang with new version..."
mv "glslang-${GLSLANG_VERSION}" glslang_new
rsync -a --delete --exclude=tnt/ glslang_new/ glslang/

echo "Cleaning up..."
rm -rf "${GLSLANG_VERSION}.zip" glslang_new

echo "Restoring LICENSE..."
git restore glslang/LICENSE

echo "Staging changes..."
git add glslang

echo "Done. Please commit the changes."
