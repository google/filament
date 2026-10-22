#version 310 es
precision mediump float;
precision highp int;

uniform mediump sampler2D SPIRV_Cross_CombineduTextureuSampler[4];
uniform mediump sampler2DArray SPIRV_Cross_CombineduTextureArrayuSampler[4];
uniform mediump samplerCube SPIRV_Cross_CombineduTextureCubeuSampler[4];
uniform mediump sampler3D SPIRV_Cross_CombineduTexture3DuSampler[4];

layout(location = 0) in vec2 vTex;
layout(location = 1) in vec3 vTex3;
layout(location = 0) out vec4 FragColor;

void main()
{
    highp vec2 _76 = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler[1], 0));
    vec2 mp_copy_76 = _76;
    highp vec2 _86 = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler[2], 1));
    vec2 mp_copy_86 = _86;
    vec2 _95 = (vTex + mp_copy_76) + mp_copy_86;
    FragColor = ((((texture(SPIRV_Cross_CombineduTextureuSampler[2], _95) + texture(SPIRV_Cross_CombineduTextureuSampler[1], _95)) + texture(SPIRV_Cross_CombineduTextureuSampler[1], _95)) + texture(SPIRV_Cross_CombineduTextureArrayuSampler[3], vTex3)) + texture(SPIRV_Cross_CombineduTextureCubeuSampler[1], vTex3)) + texture(SPIRV_Cross_CombineduTexture3DuSampler[2], vTex3);
}

