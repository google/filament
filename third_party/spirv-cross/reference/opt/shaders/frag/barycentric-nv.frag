#version 450
#extension GL_NV_fragment_shader_barycentric : require

layout(location = 0) out vec2 value;
layout(location = 0) pervertexNV in vec2 vUV[3];
layout(location = 1) pervertexNV in vec2 vUV2[3];

void main()
{
    value = ((vUV[0] * gl_BaryCoordNV.x) + (vUV[1] * gl_BaryCoordNV.y)) + (vUV[2] * gl_BaryCoordNV.z);
    value += (((vUV2[0] * gl_BaryCoordNoPerspNV.x) + (vUV2[1] * gl_BaryCoordNoPerspNV.y)) + (vUV2[2] * gl_BaryCoordNoPerspNV.z));
}

