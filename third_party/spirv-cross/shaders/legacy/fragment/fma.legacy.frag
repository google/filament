#version 450

layout(location = 0) in vec4 vA;
layout(location = 1) in vec4 vB;
layout(location = 2) in vec4 vC;
layout(location = 0) out vec4 FragColor;

void main()
{
   FragColor = fma(vA, vB, vC);
}
