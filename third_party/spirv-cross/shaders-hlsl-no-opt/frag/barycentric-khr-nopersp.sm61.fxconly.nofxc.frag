#version 450
#extension GL_EXT_fragment_shader_barycentric : require

layout(location = 0) out vec2 value;
layout(location = 1) pervertexEXT in vec2 vUV2[3];

void main () {
    value = gl_BaryCoordNoPerspEXT.x * vUV2[0] + gl_BaryCoordNoPerspEXT.y * vUV2[1] + gl_BaryCoordNoPerspEXT.z * vUV2[2];
}
