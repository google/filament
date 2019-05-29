#!/bin/bash

if [ -z "$SPIRV_CROSS_PATH" ]; then
	echo "Building spirv-cross"
	make -j$(nproc)
	SPIRV_CROSS_PATH="./spirv-cross"
fi

export PATH="./external/glslang-build/output/bin:./external/spirv-tools-build/output/bin:.:$PATH"
echo "Using glslangValidation in: $(which glslangValidator)."
echo "Using spirv-opt in: $(which spirv-opt)."
echo "Using SPIRV-Cross in: \"$SPIRV_CROSS_PATH\"."

./test_shaders.py shaders --update --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders --update --opt --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-no-opt --update --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-msl --update --msl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-msl --update --msl --opt --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-msl-no-opt --update --msl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-hlsl --update --hlsl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-hlsl --update --hlsl --opt --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-hlsl-no-opt --update --hlsl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-reflection --reflect --update --spirv-cross "$SPIRV_CROSS_PATH" || exit 1


