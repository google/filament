#version 300 es

in highp vec2 myVertex;
in mediump vec2	roadCoordsIn;

uniform highp mat4 transform;

out mediump vec2 roadCoords;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 0.0, 1.0);
	roadCoords = roadCoordsIn;
}