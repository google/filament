#version 450
layout(location = 0) out vec3 FragColor;

layout(binding = 0, std140) uniform UBO
{
	vec3 color;
	float v;
};

layout(location = 0) in vec3 vIn;

void main()
{
	FragColor = cross(vIn, color - vIn);
}
