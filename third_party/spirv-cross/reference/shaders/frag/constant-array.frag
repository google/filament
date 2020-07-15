#version 310 es
precision mediump float;
precision highp int;

struct Foobar
{
    float a;
    float b;
};

const vec4 _40[3] = vec4[](vec4(1.0), vec4(2.0), vec4(3.0));
const vec4 _57[2][2] = vec4[][](vec4[](vec4(1.0), vec4(2.0)), vec4[](vec4(8.0), vec4(10.0)));
const Foobar _77[2] = Foobar[](Foobar(10.0, 40.0), Foobar(90.0, 70.0));

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int index;

vec4 resolve(Foobar f)
{
    vec4 _25 = vec4(f.a + f.b);
    return _25;
}

void main()
{
    Foobar param = Foobar(10.0, 20.0);
    Foobar param_1 = _77[index];
    FragColor = ((_40[index] + _57[index][index + 1]) + resolve(param)) + resolve(param_1);
}

