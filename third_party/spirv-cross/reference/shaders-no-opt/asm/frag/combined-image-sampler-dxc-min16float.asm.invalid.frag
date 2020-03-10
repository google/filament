#version 310 es
precision mediump float;
precision highp int;

struct PSInput
{
    highp vec4 color;
    highp vec2 uv;
};

uniform mediump sampler2D SPIRV_Cross_CombinedtexSamp;

layout(location = 0) in highp vec4 in_var_COLOR;
layout(location = 1) in highp vec2 in_var_TEXCOORD0;
layout(location = 0) out highp vec4 out_var_SV_TARGET;

highp vec4 src_PSMain(PSInput _input)
{
    vec4 a = _input.color * texture(SPIRV_Cross_CombinedtexSamp, _input.uv);
    return a;
}

void main()
{
    PSInput param_var_input = PSInput(in_var_COLOR, in_var_TEXCOORD0);
    out_var_SV_TARGET = src_PSMain(param_var_input);
}

