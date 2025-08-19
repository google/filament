#version 460 core
#extension GL_EXT_spirv_intrinsics : enable

// ERROR: Overlapped input location. Make sure it could be detected even
// if GL_EXT_spirv_intrinsics is enabled.
layout(location = 0) in vec4 v4;
layout(location = 0) in vec3 v3;

void main() {
}
