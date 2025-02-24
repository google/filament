#version 300 es

in highp vec2 myVertex;

uniform highp mat4 transform;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 0.0, 1.0);
}