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

vec4 sample_func(vec2 uv, mediump sampler2D SPIRV_Cross_CombineduTexturesamp)
{
    return texture(SPIRV_Cross_CombineduTexturesamp, uv);
}

vec4 sample_func_dual(vec2 uv, mediump sampler2D SPIRV_Cross_Combinedtexsamp)
{
    return texture(SPIRV_Cross_Combinedtexsamp, uv);
}

void main()
{
    vec2 off = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler, 0));
    vec2 off2 = vec2(1.0) / vec2(textureSize(SPIRV_Cross_CombineduTextureuSampler, 1));
    highp vec2 param = (vTex + off) + off2;
    vec4 c0 = sample_func(param, SPIRV_Cross_CombineduTextureuSampler);
    highp vec2 param_1 = (vTex + off) + off2;
    vec4 c1 = sample_func_dual(param_1, SPIRV_Cross_CombineduTextureuSampler);
    vec4 c2 = texture(SPIRV_Cross_CombineduTextureArrayuSampler, vTex3);
    vec4 c3 = texture(SPIRV_Cross_CombineduTextureCubeuSampler, vTex3);
    vec4 c4 = texture(SPIRV_Cross_CombineduTexture3DuSampler, vTex3);
    FragColor = (((c0 + c1) + c2) + c3) + c4;
}

