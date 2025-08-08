#version 310 es
precision mediump float;
precision highp int;

struct ResType
{
    highp float _m0;
    int _m1;
};

struct ResType_1
{
    highp vec2 _m0;
    ivec2 _m1;
};

struct ResType_2
{
    highp float _m0;
    highp float _m1;
};

struct ResType_3
{
    highp vec2 _m0;
    highp vec2 _m1;
};

layout(location = 0) in float v0;
layout(location = 1) in vec2 v1;
layout(location = 0) out float FragColor;

void main()
{
    ResType _16;
    _16._m0 = frexp(v0, _16._m1);
    mediump int e0 = _16._m1;
    float f0 = _16._m0;
    ResType _22;
    _22._m0 = frexp(v0 + 1.0, _22._m1);
    e0 = _22._m1;
    f0 = _22._m0;
    ResType_1 _35;
    _35._m0 = frexp(v1, _35._m1);
    mediump ivec2 e1 = _35._m1;
    vec2 f1 = _35._m0;
    ResType_2 _42;
    _42._m0 = modf(v0, _42._m1);
    float r0 = _42._m1;
    float m0 = _42._m0;
    ResType_3 _49;
    _49._m0 = modf(v1, _49._m1);
    vec2 r1 = _49._m1;
    vec2 m1 = _49._m0;
    FragColor = ((((f0 + f1.x) + f1.y) + m0) + m1.x) + m1.y;
}

