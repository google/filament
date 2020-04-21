#version 450

layout(location = 0) in vec4 _;
layout(location = 1) in vec4 a;
layout(location = 0) out vec4 b;

void main()
{
    vec4 _28 = (_ + a) + _;
    b = _28;
    b = _;
    b = _28;
    b = _;
}

