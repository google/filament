#version 310 es
precision mediump float;

layout(location = 0) out vec2 FragColor;

void main()
{
	FragColor = gl_PointCoord;
}

