#version 450

layout(location = 0) in vec4 vA;
layout(location = 1) in vec4 vB;
layout(location = 2) in vec4 vC;

void main()
{
    precise vec4 _15 = vA * vB;
    precise vec4 _19 = vA + vB;
    precise vec4 _23 = vA - vB;
    precise vec4 _30 = _15 + vC;
    precise vec4 _34 = _15 + _19;
    precise vec4 _36 = _34 + _23;
    precise vec4 _38 = _36 + _30;
    gl_Position = _38;
}

