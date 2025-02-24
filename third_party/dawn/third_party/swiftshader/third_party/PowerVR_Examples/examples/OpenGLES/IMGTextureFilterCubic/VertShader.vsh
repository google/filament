#version 310 es

in highp vec3 inVertex;

uniform highp mat4 MVPMatrix;

out highp vec4 pos;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	pos = gl_Position;
}