#!/bin/bash

set -euo pipefail

function usage() {
  echo "Usage: $0 [-h] <version>"
  echo "  -h: Display this help message"
  echo "  <version>: The zstd version to download (e.g., 1.5.7)"
  exit 1
}

if [[ $# -eq 0 || "$1" == "-h" ]]; then
  usage
fi

ZSTD_VERSION=$1

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

echo "Downloading zstd version ${ZSTD_VERSION}..."
if ! curl -f -L -O "https://github.com/facebook/zstd/archive/refs/tags/v${ZSTD_VERSION}.zip"; then
    echo "Error: Failed to download zstd version ${ZSTD_VERSION}." >&2
    echo "Please check the version number and your network connection." >&2
    rm -f "v${ZSTD_VERSION}.zip"
    exit 1
fi

echo "Unzipping..."
if ! unzip -q "v${ZSTD_VERSION}.zip"; then
    echo "Error: Failed to unzip the downloaded file." >&2
    rm -f "v${ZSTD_VERSION}.zip"
    exit 1
fi

echo "Replacing existing zstd with new version..."
mv "zstd-${ZSTD_VERSION}" zstd_new
rsync -a --delete --exclude=tnt/ zstd_new/ zstd/

echo "Cleaning up..."
rm -rf "v${ZSTD_VERSION}.zip" zstd_new

echo "Staging changes..."
git add zstd

echo "Done. Please commit the changes."
