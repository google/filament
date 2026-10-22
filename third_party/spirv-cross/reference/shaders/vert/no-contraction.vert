#version 450

layout(location = 0) in vec4 vA;
layout(location = 1) in vec4 vB;
layout(location = 2) in vec4 vC;

void main()
{
    precise vec4 _15 = vA * vB;
    vec4 mul = _15;
    precise vec4 _19 = vA + vB;
    vec4 add = _19;
    precise vec4 _23 = vA - vB;
    vec4 sub = _23;
    precise vec4 _27 = vA * vB;
    precise vec4 _30 = _27 + vC;
    vec4 mad = _30;
    precise vec4 _34 = mul + add;
    precise vec4 _36 = _34 + sub;
    precise vec4 _38 = _36 + mad;
    vec4 summed = _38;
    gl_Position = summed;
}

