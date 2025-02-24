#version 320 es

layout(location = 0) in highp vec2 myVertex;

layout(std140, set = 0, binding = 0) uniform StaticData
{
	uniform highp mat4 transform;
};

void main(void)
{
	gl_Position = transform * vec4(myVertex, 0.0, 1.0);
}
