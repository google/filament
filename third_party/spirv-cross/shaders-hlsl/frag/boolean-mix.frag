#version 310 es
precision mediump float;

layout(location = 0) in vec2 x0;
layout(location = 0) out vec2 FragColor;

void main()
{
	FragColor = x0.x > x0.y ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
}
