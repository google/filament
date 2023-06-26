#version 450

layout(location = 0) in vec4 v;
layout(location = 0) out vec4 vo0;
layout(location = 1) out vec4 vo1;

vec4 modf_inner()
{
	return modf(v, vo1);
}

void main()
{
	vo0 = modf_inner();
}
