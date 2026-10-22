#version 450

layout(location = 0) in vec4 vA;
layout(location = 1) in float vB;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = roundEven(vA);
	FragColor *= roundEven(vB);
}
