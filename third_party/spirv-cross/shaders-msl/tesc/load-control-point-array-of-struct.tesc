#version 450

layout(vertices = 4) out;

struct VertexData
{
	mat4 a;
	vec4 b[2];
	vec4 c;
};

layout(location = 0) in VertexData vInputs[gl_MaxPatchVertices];
layout(location = 0) out vec4 vOutputs[4];

void main()
{
	VertexData tmp[gl_MaxPatchVertices] = vInputs;
	VertexData tmp_single = vInputs[gl_InvocationID ^ 1];

	vOutputs[gl_InvocationID] = tmp[gl_InvocationID].a[1] + tmp[gl_InvocationID].b[1] + tmp[gl_InvocationID].c + tmp_single.c;
}
