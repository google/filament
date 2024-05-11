#version 310 es
precision mediump float;
precision highp int;

struct Structy
{
    vec4 c;
};

layout(location = 0) out vec4 FragColor;

void foo2(inout Structy f)
{
    f.c = vec4(10.0);
}

Structy foo()
{
    Structy param;
    foo2(param);
    Structy f = param;
    return f;
}

void main()
{
    Structy s = foo();
    FragColor = s.c;
}

