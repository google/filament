#version 320 es

layout(location = 0) in highp vec2 myVertex;
layout(location = 1) in mediump vec2 texCoord;

layout(std140, set = 0, binding = 0) uniform StaticData
{
	uniform highp mat4 transform;
};

layout(location = 0) out mediump vec2 roadCoords;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 0.0, 1.0);
	roadCoords = texCoord;
}