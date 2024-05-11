#version 310 es

layout(location = 0) in vec3 pos;
layout(location = 1) in mat4 m;

void main()
{
	gl_Position = m * vec4(pos, 1.0);
}
