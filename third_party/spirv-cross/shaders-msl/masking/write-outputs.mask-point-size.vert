#version 450

layout(location = 0) out vec4 v0;
layout(location = 1) out vec4 v1;

out float gl_ClipDistance[2];

void write_in_func()
{
	v0 = vec4(1.0);
	v1 = vec4(2.0);
	gl_Position = vec4(3.0);
	gl_PointSize = 4.0;
	gl_ClipDistance[0] = 1.0;
	gl_ClipDistance[1] = 0.5;
}

void main()
{
	write_in_func();
}
