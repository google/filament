#version 450
#extension GL_EXT_texture_shadow_lod : require

layout(binding = 0) uniform sampler2DArrayShadow uShadow2DArray;
layout(binding = 1) uniform samplerCubeShadow uShadowCube;

layout(location = 0) in vec4 vUV;
layout(location = 1) in float vLod;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(((textureLod(uShadow2DArray, vec4(vUV.xyz, vUV.w), 0.0) + textureLod(uShadow2DArray, vec4(vUV.xyz, vUV.w), vLod)) + textureLod(uShadowCube, vec4(vUV.xyz, vUV.w), vLod)) + textureLodOffset(uShadow2DArray, vec4(vUV.xyz, vUV.w), vLod, ivec2(1)));
}

