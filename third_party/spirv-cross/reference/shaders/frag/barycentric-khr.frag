#version 450
#extension GL_EXT_fragment_shader_barycentric : require

layout(location = 0) out vec2 value;
layout(location = 0) pervertexEXT in vec2 vUV[3];
layout(location = 3) pervertexEXT in vec2 vUV2[3];

void main()
{
    value = ((vUV[0] * gl_BaryCoordEXT.x) + (vUV[1] * gl_BaryCoordEXT.y)) + (vUV[2] * gl_BaryCoordEXT.z);
    value += (((vUV2[0] * gl_BaryCoordNoPerspEXT.x) + (vUV2[1] * gl_BaryCoordNoPerspEXT.y)) + (vUV2[2] * gl_BaryCoordNoPerspEXT.z));
}

