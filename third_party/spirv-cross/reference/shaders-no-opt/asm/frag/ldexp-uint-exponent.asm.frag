#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out highp vec4 _GLF_color;

void main()
{
    _GLF_color = ldexp(vec4(1.0), ivec4(uvec4(bitCount(uvec4(1u)))));
}

