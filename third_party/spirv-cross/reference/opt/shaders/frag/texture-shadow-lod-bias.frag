#version 450
#extension GL_EXT_texture_shadow_lod : require

layout(binding = 0) uniform sampler2DArrayShadow uShadow2DArray;

layout(location = 0) in vec4 vUV;
layout(location = 1) in float vBias;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(uShadow2DArray, vec4(vUV.xyz, vUV.w), vBias) + textureOffset(uShadow2DArray, vec4(vUV.xyz, vUV.w), ivec2(1), vBias));
}

