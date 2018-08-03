#version 310 es
precision mediump float;
precision highp int;

struct B
{
    float a;
    float b;
};

layout(binding = 0, std140) uniform UBO
{
    mediump int some_value;
} _51;

void partial_inout(inout vec4 x)
{
    x.x = 10.0;
}

void complete_inout(out vec4 x)
{
    x = vec4(50.0);
}

void branchy_inout(inout vec4 v)
{
    v.y = 20.0;
    if (_51.some_value == 20)
    {
        v = vec4(50.0);
    }
}

void branchy_inout_2(out vec4 v)
{
    if (_51.some_value == 20)
    {
        v = vec4(50.0);
    }
    else
    {
        v = vec4(70.0);
    }
    v.y = 20.0;
}

void partial_inout(inout B b)
{
    b.b = 40.0;
}

void complete_inout(out B b)
{
    b = B(100.0, 200.0);
}

void branchy_inout(inout B b)
{
    b.b = 20.0;
    if (_51.some_value == 20)
    {
        b = B(10.0, 40.0);
    }
}

void branchy_inout_2(out B b)
{
    if (_51.some_value == 20)
    {
        b = B(10.0, 40.0);
    }
    else
    {
        b = B(70.0, 70.0);
    }
    b.b = 20.0;
}

void main()
{
    vec4 a = vec4(10.0);
    highp vec4 param = a;
    partial_inout(param);
    a = param;
    highp vec4 param_1;
    complete_inout(param_1);
    a = param_1;
    highp vec4 param_2 = a;
    branchy_inout(param_2);
    a = param_2;
    highp vec4 param_3;
    branchy_inout_2(param_3);
    a = param_3;
    B b = B(10.0, 20.0);
    B param_4 = b;
    partial_inout(param_4);
    b = param_4;
    B param_5;
    complete_inout(param_5);
    b = param_5;
    B param_6 = b;
    branchy_inout(param_6);
    b = param_6;
    B param_7;
    branchy_inout_2(param_7);
    b = param_7;
}

