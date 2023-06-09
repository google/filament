#version 450

layout(location = 0) in vec4 _;
layout(location = 1) in vec4 a;
layout(location = 0) out vec4 b;

vec4 fu_nc_(vec4 a_)
{
    return a_;
}

vec4 fu_nc_1(vec4 _0_1)
{
    return _0_1;
}

void main()
{
    vec4 b_1 = _;
    vec4 _0_1 = (_ + a) + fu_nc_(b_1);
    vec4 b_3 = a;
    vec4 b_2 = (_ - a) + fu_nc_1(b_3);
    b = _0_1;
    b = b_2;
    b = _0_1;
    b = b_2;
}

