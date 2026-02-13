#version 100
precision mediump float;
precision highp int;

struct ResType
{
    highp float _m0;
    highp float _m1;
};

struct ResType_1
{
    highp vec2 _m0;
    highp vec2 _m1;
};

varying highp float scalar;
varying highp vec2 vector;

void main()
{
    ResType _14;
    _14._m1 = float(int(scalar));
    _14._m0 = scalar - _14._m1;
    gl_FragData[0].x = _14._m1;
    gl_FragData[0].y = _14._m0;
    ResType_1 _39;
    _39._m1 = vec2(ivec2(vector));
    _39._m0 = vector - _39._m1;
    gl_FragData[0].z = _39._m0.x;
    gl_FragData[0].w = _39._m0.y;
}

