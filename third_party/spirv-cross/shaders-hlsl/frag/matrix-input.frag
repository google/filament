#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 1) in mat4 m;

void main()
{
	FragColor = m[0] + m[1] + m[2] + m[3];
}
