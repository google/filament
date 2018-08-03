#!/bin/bash

echo "Building spirv-cross"
make -j$(nproc)

export PATH="./external/glslang-build/StandAlone:./external/spirv-tools-build/tools:.:$PATH"
echo "Using glslangValidation in: $(which glslangValidator)."
echo "Using spirv-opt in: $(which spirv-opt)."

./test_shaders.py shaders || exit 1
./test_shaders.py shaders --opt || exit 1
./test_shaders.py shaders-no-opt || exit 1
./test_shaders.py shaders-msl --msl || exit 1
./test_shaders.py shaders-msl --msl --opt || exit 1
./test_shaders.py shaders-msl-no-opt --msl || exit 1
./test_shaders.py shaders-hlsl --hlsl || exit 1
./test_shaders.py shaders-hlsl --hlsl --opt || exit 1
./test_shaders.py shaders-hlsl-no-opt --hlsl || exit 1
./test_shaders.py shaders-reflection --reflect || exit 1

