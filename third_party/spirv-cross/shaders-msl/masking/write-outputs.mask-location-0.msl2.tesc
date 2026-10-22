#version 450

layout(vertices = 4) out;

layout(location = 0) out vec4 v0[];
layout(location = 1) patch out vec4 v1;

void write_in_func()
{
	v0[gl_InvocationID] = vec4(1.0);
	v0[gl_InvocationID][0] = 2.0;
	if (gl_InvocationID == 0)
	{
		v1 = vec4(2.0);
		v1[3] = 4.0;
	}
	gl_out[gl_InvocationID].gl_Position = vec4(3.0);
	gl_out[gl_InvocationID].gl_PointSize = 4.0;
	gl_out[gl_InvocationID].gl_Position[2] = 5.0;
	gl_out[gl_InvocationID].gl_PointSize = 4.0;
}

void main()
{
	write_in_func();
}
