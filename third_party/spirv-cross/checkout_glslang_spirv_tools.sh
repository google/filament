#!/bin/bash
# Copyright 2016-2021 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

GLSLANG_REV=df609a01b386001e367709086c58529c48028d1e
SPIRV_TOOLS_REV=75e53b9f685830ac42242cf0c46cc9af523bd0df
SPIRV_HEADERS_REV=b8047fbe45f426f5918fadc67e8408f5b108c3c9
PROTOCOL=https

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

