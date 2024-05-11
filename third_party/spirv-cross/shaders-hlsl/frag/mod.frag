#version 310 es
precision mediump float;

layout(location = 0) in vec4 a4;
layout(location = 1) in vec3 a3;
layout(location = 2) in vec2 a2;
layout(location = 3) in float a1;
layout(location = 4) in vec4 b4;
layout(location = 5) in vec3 b3;
layout(location = 6) in vec2 b2;
layout(location = 7) in float b1;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 m0 = mod(a4, b4);
	vec3 m1 = mod(a3, b3);
	vec2 m2 = mod(a2, b2);
	float m3 = mod(a1, b1);
	FragColor = m0 + m1.xyzx + m2.xyxy + m3;
}
