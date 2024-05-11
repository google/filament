#version 310 es

layout(location=0) in ivec4 attr_int4;
layout(location=1) in int attr_int1;

void main()
{
	gl_Position.x = float(attr_int4[attr_int1]);
	gl_Position.yzw = vec3(0.0f, 0.0f, 0.0f);
}
