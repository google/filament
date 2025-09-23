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
    ResType _22;
    _22._m0 = frexp(v0 + 1.0, _22._m1);
    highp float _24 = _22._m0;
    float mp_copy_24 = _24;
    ResType_1 _35;
    _35._m0 = frexp(v1, _35._m1);
    ResType_2 _42;
    _42._m0 = modf(v0, _42._m1);
    ResType_3 _49;
    _49._m0 = modf(v1, _49._m1);
    FragColor = ((((mp_copy_24 + _35._m0.x) + _35._m0.y) + _42._m0) + _49._m0.x) + _49._m0.y;
}

