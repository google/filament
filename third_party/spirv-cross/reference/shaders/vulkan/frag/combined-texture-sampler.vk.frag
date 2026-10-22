#version 310 es
precision mediump float;
precision highp int;

uniform mediump sampler2D SPIRV_Cross_CombineduTexture0uSampler0;
uniform mediump sampler2D SPIRV_Cross_CombineduTexture1uSampler1;
uniform mediump sampler2D SPIRV_Cross_CombineduTexture1uSampler0;
uniform mediump sampler2D SPIRV_Cross_CombineduTexture0uSampler1;

layout(location = 0) in vec2 vTex;
layout(location = 0) out vec4 FragColor;

vec4 sample_dual(mediump sampler2D SPIRV_Cross_Combinedtexsamp)
{
    return texture(SPIRV_Cross_Combinedtexsamp, vTex);
}

vec4 sample_duals()
{
    vec4 a = sample_dual(SPIRV_Cross_CombineduTexture0uSampler0);
    vec4 b = sample_dual(SPIRV_Cross_CombineduTexture1uSampler1);
    return a + b;
}

vec4 sample_global_tex(mediump sampler2D SPIRV_Cross_CombineduTexture0samp, mediump sampler2D SPIRV_Cross_CombineduTexture1samp)
{
    vec4 a = texture(SPIRV_Cross_CombineduTexture0samp, vTex);
    vec4 b = sample_dual(SPIRV_Cross_CombineduTexture1samp);
    return a + b;
}

vec4 sample_global_sampler(mediump sampler2D SPIRV_Cross_CombinedtexuSampler0, mediump sampler2D SPIRV_Cross_CombinedtexuSampler1)
{
    vec4 a = texture(SPIRV_Cross_CombinedtexuSampler0, vTex);
    vec4 b = sample_dual(SPIRV_Cross_CombinedtexuSampler1);
    return a + b;
}

void main()
{
    vec4 c0 = sample_duals();
    vec4 c1 = sample_global_tex(SPIRV_Cross_CombineduTexture0uSampler0, SPIRV_Cross_CombineduTexture1uSampler0);
    vec4 c2 = sample_global_tex(SPIRV_Cross_CombineduTexture0uSampler1, SPIRV_Cross_CombineduTexture1uSampler1);
    vec4 c3 = sample_global_sampler(SPIRV_Cross_CombineduTexture0uSampler0, SPIRV_Cross_CombineduTexture0uSampler1);
    vec4 c4 = sample_global_sampler(SPIRV_Cross_CombineduTexture1uSampler0, SPIRV_Cross_CombineduTexture1uSampler1);
    FragColor = (((c0 + c1) + c2) + c3) + c4;
}

