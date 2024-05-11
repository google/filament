#version 450

layout(location = 0) in vec4 v;
layout(location = 0) out vec4 vo0;
layout(location = 1) out vec4 vo1;

vec4 modf_inner(out vec4 tmp)
{
	return modf(v, tmp);
}

float modf_inner_partial(inout vec4 tmp)
{
	return modf(v.x, tmp.x);
}

void main()
{
	vec4 tmp;
	vo0 = modf_inner(tmp);
	vo1 = tmp;

	vo0.x += modf_inner_partial(tmp);
	vo1.x += tmp.x;
}
