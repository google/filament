#version 450
layout(location = 0) out vec4 FragColor;
layout(binding = 0, std430) readonly buffer SSBO
{
	vec4 v;
};

vec4 read_from_function()
{
	return v;
}

void main()
{
	FragColor = v + read_from_function();
}
