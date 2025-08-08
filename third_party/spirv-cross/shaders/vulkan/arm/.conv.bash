#!/usr/bin/bash

FILES=$(ls tensor*)

for FILE in ${FILES}; do
  ${REPO}/dependencies/glslang/build/StandAlone/glslang ${FILE} -V --target-env vulkan1.1 -o asm/${FILE}.spv
  ${REPO}/dependencies/SPIRV-Tools/build/tools/spirv-dis asm/${FILE}.spv -o asm/${FILE}.spvasm
  rm asm/${FILE}.spv
done
