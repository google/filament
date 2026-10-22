#version 310 es
precision mediump float;

layout(location = 0) in vec2 value;

layout(location = 0) out vec4 FragColor;

void main()
{
	int i = floatBitsToInt(value.x);
	FragColor = vec4(1.0, 0.0, intBitsToFloat(i), 1.0);
}
