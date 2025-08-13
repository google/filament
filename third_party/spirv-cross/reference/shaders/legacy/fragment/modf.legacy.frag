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
    highp float sipart = _14._m1;
    highp float sfpart = _14._m0;
    highp vec2 _23 = vec2(sipart, sfpart);
    gl_FragData[0].x = _23.x;
    gl_FragData[0].y = _23.y;
    ResType_1 _39;
    _39._m1 = vec2(ivec2(vector));
    _39._m0 = vector - _39._m1;
    highp vec2 vipart = _39._m1;
    highp vec2 vfpart = _39._m0;
    gl_FragData[0].z = vfpart.x;
    gl_FragData[0].w = vfpart.y;
}

