#version 450

layout(vertices = 4) out;

layout(location = 0) out vec4 v0[];
layout(location = 1) patch out vec4 v1[2];
layout(location = 3) patch out vec4 v3;

void write_in_func()
{
	v0[gl_InvocationID] = vec4(1.0);
	v0[gl_InvocationID].z = 3.0;
	if (gl_InvocationID == 0)
	{
		v1[0] = vec4(2.0);
		v1[0].x = 3.0;
		v1[1] = vec4(2.0);
		v1[1].x = 5.0;
	}
	v3 = vec4(5.0);
	gl_out[gl_InvocationID].gl_Position = vec4(10.0);
	gl_out[gl_InvocationID].gl_Position.z = 20.0;
	gl_out[gl_InvocationID].gl_PointSize = 40.0;
}

void main()
{
	write_in_func();
}
