#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out highp vec4 _GLF_color;

void main()
{
    mediump uvec4 _18 = uvec4(bitCount(uvec4(1u)));
    uvec4 hp_copy_18 = _18;
    _GLF_color = ldexp(vec4(1.0), ivec4(hp_copy_18));
}

