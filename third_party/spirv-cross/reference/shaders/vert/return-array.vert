#version 310 es

layout(location = 0) in vec4 vInput0;
layout(location = 1) in vec4 vInput1;

vec4[2] test()
{
    return vec4[](vec4(10.0), vec4(20.0));
}

vec4[2] test2()
{
    vec4 foobar[2];
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    return foobar;
}

void main()
{
    gl_Position = test()[0] + test2()[1];
}

