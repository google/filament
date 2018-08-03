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

vec4 sample_func(vec2 uv, mediump sampler2D SPIRV_Cross_CombineduTexturesamp[4])
{
    return texture(SPIRV_Cross_CombineduTexturesamp[2], uv);
}

vec4 sample_func_dual(vec2 uv, mediump sampler2D SPIRV_Cross_Combinedtexsamp)
{
    return texture(SPIRV_Cross_Combinedtexsamp, uv);
}

vec4 sample_func_dual_array(vec2 uv, mediump sampler2D SPIRV_Cross_Combinedtexsamp[4])
{
    return texture(SPIRV_Cross_Combinedtexsamp[1], uv);
}

void main()
{
    vec2 off = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler[1], 0));
    vec2 off2 = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler[2], 1));
    highp vec2 param = (vTex + off) + off2;
    vec4 c0 = sample_func(param, SPIRV_Cross_CombineduTextureuSampler);
    highp vec2 param_1 = (vTex + off) + off2;
    vec4 c1 = sample_func_dual(param_1, SPIRV_Cross_CombineduTextureuSampler[1]);
    highp vec2 param_2 = (vTex + off) + off2;
    vec4 c2 = sample_func_dual_array(param_2, SPIRV_Cross_CombineduTextureuSampler);
    vec4 c3 = texture(SPIRV_Cross_CombineduTextureArrayuSampler[3], vTex3);
    vec4 c4 = texture(SPIRV_Cross_CombineduTextureCubeuSampler[1], vTex3);
    vec4 c5 = texture(SPIRV_Cross_CombineduTexture3DuSampler[2], vTex3);
    FragColor = ((((c0 + c1) + c2) + c3) + c4) + c5;
}

