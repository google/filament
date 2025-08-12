#version 310 es
precision mediump float;
precision highp int;

layout(binding = 1, std140) uniform Count
{
    float count;
} _44;

layout(binding = 0) uniform mediump sampler2D tex;

layout(location = 0) in highp vec4 vertex;
layout(location = 0) out vec4 fragColor;

void main()
{
    highp float _24 = 1.0 / float(textureSize(tex, 0).x);
    highp float _34 = dFdx(vertex.x);
    float _62;
    _62 = 0.0;
    for (float _61 = 0.0; _61 < _44.count; )
    {
        _62 = _24 * _34 + _62;
        _61 += 1.0;
        continue;
    }
    fragColor = vec4(_62);
}

