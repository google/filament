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

layout(location = 0) in float v0;
layout(location = 1) in vec2 v1;
layout(location = 0) out float FragColor;

void main()
{
    ResType _22;
    _22._m0 = frexp(v0 + 1.0, _22._m1);
    ResType_1 _35;
    _35._m0 = frexp(v1, _35._m1);
    float r0;
    float _41 = modf(v0, r0);
    vec2 r1;
    vec2 _45 = modf(v1, r1);
    FragColor = ((((_22._m0 + _35._m0.x) + _35._m0.y) + _41) + _45.x) + _45.y;
}

