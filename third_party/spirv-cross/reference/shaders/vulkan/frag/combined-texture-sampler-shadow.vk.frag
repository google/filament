#version 310 es
precision mediump float;
precision highp int;

uniform mediump sampler2DShadow SPIRV_Cross_CombineduDepthuSampler;
uniform mediump sampler2D SPIRV_Cross_CombineduDepthuSampler1;

layout(location = 0) out float FragColor;

float samp2(mediump sampler2DShadow SPIRV_Cross_Combinedts)
{
    return texture(SPIRV_Cross_Combinedts, vec3(vec3(1.0).xy, 1.0));
}

float samp3(mediump sampler2D SPIRV_Cross_Combinedts)
{
    return texture(SPIRV_Cross_Combinedts, vec2(1.0)).x;
}

float samp(mediump sampler2DShadow SPIRV_Cross_Combinedts, mediump sampler2D SPIRV_Cross_Combinedts1)
{
    float r0 = samp2(SPIRV_Cross_Combinedts);
    float r1 = samp3(SPIRV_Cross_Combinedts1);
    return r0 + r1;
}

void main()
{
    FragColor = samp(SPIRV_Cross_CombineduDepthuSampler, SPIRV_Cross_CombineduDepthuSampler1);
}

