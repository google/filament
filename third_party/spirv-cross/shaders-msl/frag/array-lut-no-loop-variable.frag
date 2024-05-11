#version 310 es
precision mediump float;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 v0;

void main()
{
	float lut[5] = float[](1.0, 2.0, 3.0, 4.0, 5.0);
	for (int i = 0; i < 4; i++, FragColor += lut[i])
	{
	}
}
