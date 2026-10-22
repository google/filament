#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 fragment = vec4(10.0);
	vec4 compute = vec4(10.0);
	vec4 kernel = vec4(10.0);
	vec4 vertex = vec4(10.0);
	FragColor = fragment + compute + kernel + vertex;
}
