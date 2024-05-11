#version 450

layout(vertices = 4) out;

layout(location = 0) in vec4 vInputs[gl_MaxPatchVertices];
layout(location = 0) out vec4 vOutputs[4];

void main()
{
	vec4 tmp[gl_MaxPatchVertices] = vInputs;
	vOutputs[gl_InvocationID] = tmp[gl_InvocationID];
}
