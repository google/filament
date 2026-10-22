#version 310 es
precision mediump float;
layout(location = 0) out vec3 FragColor;
layout(location = 0) flat in vec3 vNormal;

layout(binding = 0, std140) uniform UBO
{
	mat4 m;
};

void main()
{
	FragColor = mat3(m) * vNormal;
}
