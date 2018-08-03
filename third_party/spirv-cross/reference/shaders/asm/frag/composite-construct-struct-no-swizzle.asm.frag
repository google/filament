#version 310 es
precision mediump float;
precision highp int;

struct SwizzleTest
{
    float a;
    float b;
};

layout(location = 0) in vec2 foo;
layout(location = 0) out float FooOut;

void main()
{
    SwizzleTest _22 = SwizzleTest(foo.x, foo.y);
    FooOut = _22.a + _22.b;
}

