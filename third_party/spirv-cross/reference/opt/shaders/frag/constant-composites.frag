#version 310 es
precision mediump float;
precision highp int;

const float _16[4] = float[](1.0, 4.0, 3.0, 2.0);

struct Foo
{
    float a;
    float b;
};

const Foo _28[2] = Foo[](Foo(10.0, 20.0), Foo(30.0, 40.0));

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int line;

void main()
{
    FragColor = vec4(_16[line]);
    FragColor += vec4(_28[line].a * _28[1 - line].a);
}

