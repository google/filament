#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vA;
layout(location = 1) in float vB;

void main()
{
    FragColor = roundEven(vA);
    FragColor *= roundEven(vB);
}

