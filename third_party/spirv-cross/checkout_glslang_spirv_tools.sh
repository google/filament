#!/bin/bash

GLSLANG_REV=f5ed7a69d5d64bd3ac802712c24995c6c12d23f8
SPIRV_TOOLS_REV=bd2a9ea85210d3bb474bc5adb9ff4b0bb536b4fc
SPIRV_HEADERS_REV=11d7637e7a43cd88cfd4e42c99581dcb682936aa

if [ -z $PROTOCOL ]; then
	PROTOCOL=git
fi

echo "Using protocol \"$PROTOCOL\" for checking out repositories. If this is problematic, try PROTOCOL=https $0."

if [ -d external/glslang ]; then
	echo "Updating glslang to revision $GLSLANG_REV."
	cd external/glslang
	git fetch origin
	git checkout $GLSLANG_REV
else
	echo "Cloning glslang revision $GLSLANG_REV."
	mkdir -p external
	cd external
	git clone $PROTOCOL://github.com/KhronosGroup/glslang.git
	cd glslang
	git checkout $GLSLANG_REV
fi
cd ../..

if [ -d external/spirv-tools ]; then
	echo "Updating SPIRV-Tools to revision $SPIRV_TOOLS_REV."
	cd external/spirv-tools
	git fetch origin
	git checkout $SPIRV_TOOLS_REV
else
	echo "Cloning SPIRV-Tools revision $SPIRV_TOOLS_REV."
	mkdir -p external
	cd external
	git clone $PROTOCOL://github.com/KhronosGroup/SPIRV-Tools.git spirv-tools
	cd spirv-tools
	git checkout $SPIRV_TOOLS_REV
fi

if [ -d external/spirv-headers ]; then
	cd external/spirv-headers
	git pull origin master
	git checkout $SPIRV_HEADERS_REV
	cd ../..
else
	git clone $PROTOCOL://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers
	cd external/spirv-headers
	git checkout $SPIRV_HEADERS_REV
	cd ../..
fi

cd ../..

