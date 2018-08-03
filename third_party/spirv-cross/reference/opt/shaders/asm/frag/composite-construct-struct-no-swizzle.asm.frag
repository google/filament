#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) in vec2 foo;
layout(location = 0) out float FooOut;

void main()
{
    FooOut = foo.x + foo.y;
}

