#version 450

layout(set = 0, binding = 0) coherent readonly buffer SSBO
{
	vec4 a;
};
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = a;
}
