#!/bin/bash

PROFILE=Release
if [ ! -z $1 ]; then
	PROFILE=$1
fi

NPROC=$(nproc)
if [ ! -z $2 ]; then
	NPROC=$2
fi

echo "Building glslang."
mkdir -p external/glslang-build
cd external/glslang-build
cmake ../glslang -DCMAKE_BUILD_TYPE=$PROFILE -G"Unix Makefiles"
make -j$NPROC
cd ../..

echo "Building SPIRV-Tools."
mkdir -p external/spirv-tools-build
cd external/spirv-tools-build
cmake ../spirv-tools -DCMAKE_BUILD_TYPE=$PROFILE -G"Unix Makefiles" -DSPIRV_WERROR=OFF
make -j$NPROC
cd ../..

