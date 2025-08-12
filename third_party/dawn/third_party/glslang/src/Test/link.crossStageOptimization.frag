#version 440

layout(location = 0) in vec4 a0; // accessed
layout(location = 1) in vec4 a1; // not accessed
layout(location = 2) in vec4 a2; // accessed
layout(location = 3) in vec4 a3; // not accessed

layout(location = 0) out vec4 oColor;

void main()
{
    vec4 temp = vec4(1.0);
    if (true)
    {
        temp *= a0;
        temp *= a2;
    }
    if (false)
    {
        temp *= a1;
        temp *= a3;
    }
    oColor = temp;
}
