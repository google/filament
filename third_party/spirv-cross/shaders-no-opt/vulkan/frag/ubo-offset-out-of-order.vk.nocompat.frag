#version 450

layout(location = 0) out vec4 FragColor;

layout(std140, binding = 0) uniform UBO
{
	layout(offset = 16) mat4 m;
	layout(offset = 0) vec4 v;
};

layout(location = 0) in vec4 vColor;

void main()
{
	FragColor = m * vColor + v;
}
