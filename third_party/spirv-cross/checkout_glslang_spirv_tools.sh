#!/bin/bash

GLSLANG_REV=4dcc12d1a441b29d5901bc708bb1343d29d6459f
SPIRV_TOOLS_REV=0a1fb588cd365f7737cb121fdd64553923e0cef6
SPIRV_HEADERS_REV=060627f0b0d2fa8581b5acb939f46e3b9e500593

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

