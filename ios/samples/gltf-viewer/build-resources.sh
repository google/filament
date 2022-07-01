#/usr/bin/env/bash

set -e

# Compile resources.

# The gltf-viewer app requires two resources:
# 1. The IBL image
# 2. The skybox image

HOST_TOOLS_PATH="${HOST_TOOLS_PATH:-../../../out/release/filament/bin}"

cmgen_path=`find ${HOST_TOOLS_PATH} -name cmgen -type f | head -n 1`

# Ensure that the required tools are present in the out/ directory.
# These can be built by running ./build.sh -p desktop -i release at Filament's root directory.

if [[ ! -e "${cmgen_path}" ]]; then
  echo "No cmgen binary could be found in ${HOST_TOOLS_PATH}."
  echo "Ensure Filament has been built/installed before building this app."
  exit 1
fi

# cmgen consumes an HDR environment map and generates two mipmapped KTX files (IBL and skybox)
"${cmgen_path}" \
    --quiet \
    --deploy="${PROJECT_DIR}/generated/default_env" \
    --format=ktx --size=256 --extract-blur=0.1 \
    "${PROJECT_DIR}/../../../third_party/environments/lightroom_14b.hdr"
