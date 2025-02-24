#version 310 es

in highp vec3 inVertex;

uniform highp mat4 MVPMatrix;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}