#version 450

#extension GL_GOOGLE_include_directive : require
#include "spv.debuginfo.include.glsl.h"

vec4 mainFileFunction(vec4 v) {
	return -v;
}

void main() {
	headerOut = headerFunction(mainFileFunction(headerUboItem));
}