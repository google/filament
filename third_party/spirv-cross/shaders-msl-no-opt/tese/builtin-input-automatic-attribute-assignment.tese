#version 450
layout(quads) in;

layout(location = 0) patch in vec4 FragColor;
layout(location = 2) in vec4 FragColors[];

void main()
{
	gl_Position = vec4(1.0) + FragColor + FragColors[0] + FragColors[1] + gl_TessLevelInner[0] + gl_TessLevelOuter[gl_PrimitiveID & 1] + gl_in[0].gl_Position;
}
