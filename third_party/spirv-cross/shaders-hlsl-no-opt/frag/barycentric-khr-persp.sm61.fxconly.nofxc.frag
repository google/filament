#version 450
#extension GL_EXT_fragment_shader_barycentric : require

layout(location = 0) out vec2 value;
layout(location = 0) pervertexEXT in vec2 vUV[3];

void main () {
    value = gl_BaryCoordEXT.x * vUV[0] + gl_BaryCoordEXT.y * vUV[1] + gl_BaryCoordEXT.z * vUV[2];
}
