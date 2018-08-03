#version 310 es
precision highp float;

const float posinf = 1.0 / 0.0;
const float neginf = -1.0 / 0.0;
const float nan = 0.0 / 0.0;

layout(location = 0) out vec3 FragColor;

void main()
{
	FragColor = vec3(posinf, neginf, nan);
}

