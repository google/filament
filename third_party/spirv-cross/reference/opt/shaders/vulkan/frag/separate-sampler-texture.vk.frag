#version 310 es
precision mediump float;
precision highp int;

uniform mediump sampler2D SPIRV_Cross_CombineduTextureuSampler;
uniform mediump sampler2DArray SPIRV_Cross_CombineduTextureArrayuSampler;
uniform mediump samplerCube SPIRV_Cross_CombineduTextureCubeuSampler;
uniform mediump sampler3D SPIRV_Cross_CombineduTexture3DuSampler;

layout(location = 0) in vec2 vTex;
layout(location = 1) in vec3 vTex3;
layout(location = 0) out vec4 FragColor;

void main()
{
    highp vec2 _54 = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler, 0));
    vec2 mp_copy_54 = _54;
    highp vec2 _64 = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler, 1));
    vec2 mp_copy_64 = _64;
    vec2 _73 = (vTex + mp_copy_54) + mp_copy_64;
    FragColor = (((texture(SPIRV_Cross_CombineduTextureuSampler, _73) + texture(SPIRV_Cross_CombineduTextureuSampler, _73)) + texture(SPIRV_Cross_CombineduTextureArrayuSampler, vTex3)) + texture(SPIRV_Cross_CombineduTextureCubeuSampler, vTex3)) + texture(SPIRV_Cross_CombineduTexture3DuSampler, vTex3);
}

