#version 450
layout(constant_id = 0) const int ARR_SIZE = 1;

layout(binding = 0, set = 1, std140) uniform u_
{
	vec4 u_0[ARR_SIZE];
};

void main()
{
	gl_Position = u_0[0];
}

