#!/bin/bash
# Copyright 2016-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

OPTS=$@

if [ -z "$SPIRV_CROSS_PATH" ]; then
	echo "Building spirv-cross"
	make -j$(nproc)
	SPIRV_CROSS_PATH="./spirv-cross"
fi

export PATH="./external/glslang-build/output/bin:./external/spirv-tools-build/output/bin:.:$PATH"
echo "Using glslangValidation in: $(which glslangValidator)."
echo "Using spirv-opt in: $(which spirv-opt)."
echo "Using SPIRV-Cross in: \"$SPIRV_CROSS_PATH\"."

./test_shaders.py shaders ${OPTS} --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders ${OPTS} --opt --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-no-opt ${OPTS} --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-msl ${OPTS} --msl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-msl ${OPTS} --msl --opt --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-msl-no-opt ${OPTS} --msl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-hlsl ${OPTS} --hlsl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-hlsl ${OPTS} --hlsl --opt --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-hlsl-no-opt ${OPTS} --hlsl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-reflection ${OPTS} --reflect --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-ue4 ${OPTS} --msl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-ue4 ${OPTS} --msl --opt --spirv-cross "$SPIRV_CROSS_PATH" || exit 1
./test_shaders.py shaders-ue4-no-opt ${OPTS} --msl --spirv-cross "$SPIRV_CROSS_PATH" || exit 1

