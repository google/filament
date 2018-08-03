#!/bin/bash

echo "Building spirv-cross"
make -j$(nproc)

export PATH="./external/glslang-build/StandAlone:./external/spirv-tools-build/tools:.:$PATH"
echo "Using glslangValidation in: $(which glslangValidator)."
echo "Using spirv-opt in: $(which spirv-opt)."

./test_shaders.py shaders --update || exit 1
./test_shaders.py shaders --update --opt || exit 1
./test_shaders.py shaders-no-opt --update || exit 1
./test_shaders.py shaders-msl --update --msl || exit 1
./test_shaders.py shaders-msl --update --msl --opt || exit 1
./test_shaders.py shaders-msl-no-opt --update --msl || exit 1
./test_shaders.py shaders-hlsl --update --hlsl || exit 1
./test_shaders.py shaders-hlsl --update --hlsl --opt || exit 1
./test_shaders.py shaders-hlsl-no-opt --update --hlsl || exit 1
./test_shaders.py shaders-reflection --reflect --update || exit 1


