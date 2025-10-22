#!/usr/bin/env bash

set -ex

# Run from Filament's root.
# This script updates the test material used to verify old materials can still be parsed.
# See filament_test_material_parser.cpp.

OUTPUT_DIR="filament/test/"

./build.sh -p desktop release matc
MATC=out/cmake-release/tools/matc/matc
${MATC} --platform all --api all -o ${OUTPUT_DIR}/test_material.filamat samples/materials/sandboxLit.mat
${MATC} --platform all --api all -o ${OUTPUT_DIR}/test_material_transformname.filamat samples/materials/sandboxTransformName.mat

set +x

echo "===================================================================="
echo "Test material: filament/test/test_material.filamat has been updated."
echo "Re-build and run test_material_parser and to verify."
echo "Then commit and push changes."
echo "===================================================================="
