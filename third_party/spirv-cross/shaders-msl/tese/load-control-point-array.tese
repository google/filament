#version 450

layout(cw, quads) in;
layout(location = 0) in vec4 vInputs[gl_MaxPatchVertices];
layout(location = 1) patch in vec4 vBoo[4];
layout(location = 5) patch in int vIndex;

void main()
{
	vec4 tmp[gl_MaxPatchVertices] = vInputs;
	gl_Position = tmp[0] + tmp[1] + vBoo[vIndex];

}
