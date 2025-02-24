#version 320 es

#define VERTEX_ARRAY	0
layout(location = VERTEX_ARRAY) in highp vec4 inVertex;

layout(set = 0, binding = 0) uniform MVP
{
	highp mat4 MVPMatrix;
};

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex.rgb, 1.0);
}