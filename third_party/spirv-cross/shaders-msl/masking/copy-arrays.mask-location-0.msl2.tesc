#version 450

layout(vertices = 4) out;
layout(location = 0) out vec4 Foo[][2];
layout(location = 0) in vec4 iFoo[][2];
layout(location = 2) patch out vec4 pFoo[2];
layout(location = 2) in vec4 ipFoo[];

void main()
{
	gl_out[gl_InvocationID].gl_Position = vec4(1.0);
	Foo[gl_InvocationID] = iFoo[gl_InvocationID];
	if (gl_InvocationID == 0)
	{
		pFoo = vec4[](ipFoo[0], ipFoo[1]);
	}
}
