# Compile bakedColor material.

HOST_TOOLS_PATH="${HOST_TOOLS_PATH:-../../../out/release/filament/bin}"
matc_path=`find ${HOST_TOOLS_PATH} -name matc -type f | head -n 1`

if [[ ! -e "${matc_path}" ]]; then
  echo "No matc binary could be found in ${HOST_TOOLS_PATH}."
  echo "Ensure Filament has been built/installed before building this app."
  exit 1
fi

mkdir -p "${PROJECT_DIR}/generated/"
"${matc_path}" \
    --api all \
    -f header \
    -o "${PROJECT_DIR}/generated/bakedColor.inc" \
    "${PROJECT_DIR}/Materials/bakedColor.mat"

# FilamentView.mm includes bakedColor.filamat, so touch it to force Xcode to
# recompile it.
touch "${PROJECT_DIR}/hello-triangle/FilamentView.mm"
