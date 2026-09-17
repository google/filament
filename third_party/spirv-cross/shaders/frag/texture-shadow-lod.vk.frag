#version 450
#extension GL_EXT_texture_shadow_lod : require

layout(binding = 0) uniform sampler2DArrayShadow uShadow2DArray;
layout(binding = 1) uniform samplerCubeShadow uShadowCube;

layout(location = 0) in vec4 vUV;
layout(location = 1) in float vLod;
layout(location = 0) out vec4 FragColor;

void main()
{
    float r = 0.0;
    r += textureLod(uShadow2DArray, vUV, 0.0);
    r += textureLod(uShadow2DArray, vUV, vLod);
    r += textureLod(uShadowCube, vUV, vLod);
    r += textureLodOffset(uShadow2DArray, vUV, vLod, ivec2(1, 1));
    FragColor = vec4(r);
}
