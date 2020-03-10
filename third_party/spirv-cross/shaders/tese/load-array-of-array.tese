#version 450
layout(ccw, quads) in;

layout(location = 0) in vec4 vTexCoord[][1];

void main()
{
	vec4 tmp[gl_MaxPatchVertices][1] = vTexCoord;
	gl_Position = tmp[0][0] + tmp[2][0] + tmp[3][0];
}
