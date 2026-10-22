#version 450
#extension GL_EXT_texture_shadow_lod : require

layout(binding = 0) uniform sampler2DArrayShadow uShadow2DArray;

layout(location = 0) in vec4 vUV;
layout(location = 1) in float vBias;
layout(location = 0) out vec4 FragColor;

void main()
{
    float r = 0.0;
    r += texture(uShadow2DArray, vUV, vBias);
    r += textureOffset(uShadow2DArray, vUV, ivec2(1, 1), vBias);
    FragColor = vec4(r);
}
