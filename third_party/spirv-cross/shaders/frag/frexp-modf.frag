#version 310 es
precision mediump float;

layout(location = 0) out float FragColor;
layout(location = 0) in float v0;
layout(location = 1) in vec2 v1;

void main()
{
	int e0;
	float f0 = frexp(v0, e0);
	f0 = frexp(v0 + 1.0, e0);

	ivec2 e1;
	vec2 f1 = frexp(v1, e1);

	float r0;
	float m0 = modf(v0, r0);
	vec2 r1;
	vec2 m1 = modf(v1, r1);

	FragColor = f0 + f1.x + f1.y + m0 + m1.x + m1.y;
}

