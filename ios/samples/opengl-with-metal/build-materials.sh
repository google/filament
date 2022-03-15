#!/usr/bin/env bash

set -e

# Compile SampleExternalTexture material.

HOST_TOOLS_PATH="${HOST_TOOLS_PATH:-../../../out/release/filament/bin}"
matc_path=`find ${HOST_TOOLS_PATH} -name matc -type f | head -n 1`

if [[ ! -e "${matc_path}" ]]; then
  echo "error: No matc binary could be found in ${HOST_TOOLS_PATH}."
  echo "error: Ensure Filament has been built/installed before building this app."
  exit 1
fi

mkdir -p "${PROJECT_DIR}/generated/"
"${matc_path}" \
    --api all \
    -f header \
    -o "${PROJECT_DIR}/generated/SampleExternalTexture.inc" \
    "${PROJECT_DIR}/Materials/SampleExternalTexture.mat"

# FilamentView.mm includes the material, so touch it to force Xcode to recompile it.
touch "${PROJECT_DIR}/opengl-with-metal/FilamentView.mm"
