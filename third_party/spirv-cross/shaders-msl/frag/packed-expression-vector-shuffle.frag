#version 450
layout(location = 0) out vec4 FragColor;

layout(binding = 0, std140) uniform UBO
{
	vec3 color;
	float v;
};

void main()
{
	vec4 f = vec4(1.0);
	f.rgb = color;
	FragColor = f;
}
