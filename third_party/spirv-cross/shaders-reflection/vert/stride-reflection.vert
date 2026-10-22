#version 450

layout(binding = 0, set = 0, std140) uniform U
{
	vec4 v[4];
	mat4 c[4];
	layout(row_major) mat4 r[4];
};

void main()
{
	gl_Position = v[0];
}

