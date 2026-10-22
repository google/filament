#version 450

layout(vertices = 4) out;

layout(location = 0) in mat4 vInputs[gl_MaxPatchVertices];
layout(location = 0) out mat4 vOutputs[4];

void main()
{
	mat4 tmp[gl_MaxPatchVertices] = vInputs;
	vOutputs[gl_InvocationID] = tmp[gl_InvocationID];
}
