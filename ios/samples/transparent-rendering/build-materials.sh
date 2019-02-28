# Compile bakedColor material.
matc_path="../../../out/release/filament/bin/matc"
if [[ ! -e "${matc_path}" ]]; then
  echo "No matc binary could be found in ../../../out/release/filament/bin/."
  echo "Ensure Filament has been built/installed before building this app."
  exit 1
fi
mkdir -p "${PROJECT_DIR}/generated/"
"${matc_path}" \
    --api all \
    -f header \
    -o "${PROJECT_DIR}/generated/bakedColor.inc" \
    "${PROJECT_DIR}/transparent-rendering/bakedColor.mat"

# FilamentView.mm includes bakedColor.filamat, so touch it to force Xcode to
# recompile it.
touch "${PROJECT_DIR}/transparent-rendering/FilamentView.mm"
