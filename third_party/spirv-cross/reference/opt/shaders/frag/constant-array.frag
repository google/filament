#version 310 es
precision mediump float;
precision highp int;

struct Foobar
{
    float a;
    float b;
};

const vec4 _37[3] = vec4[](vec4(1.0), vec4(2.0), vec4(3.0));
const vec4 _55[2][2] = vec4[][](vec4[](vec4(1.0), vec4(2.0)), vec4[](vec4(8.0), vec4(10.0)));
const Foobar _75[2] = Foobar[](Foobar(10.0, 40.0), Foobar(90.0, 70.0));

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int index;

void main()
{
    highp float _106 = _75[index].a;
    float mp_copy_106 = _106;
    highp float _107 = _75[index].b;
    float mp_copy_107 = _107;
    FragColor = ((_37[index] + _55[index][index + 1]) + vec4(30.0)) + vec4(mp_copy_106 + mp_copy_107);
}

