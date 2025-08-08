#version 450

layout(cw, quads) in;
layout(location = 0) in mat4 vInputs[gl_MaxPatchVertices];
layout(location = 4) patch in vec4 vBoo[4];
layout(location = 8) patch in int vIndex;

void main()
{
	mat4 tmp[gl_MaxPatchVertices] = vInputs;
	gl_Position = tmp[0][vIndex] + tmp[1][vIndex] + vBoo[vIndex];

}
