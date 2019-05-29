#version 450

layout(std140, binding = 4) uniform CBO
{
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
} cbo[2][4];

layout(std430, push_constant) uniform PushMe
{
	vec4 a;
	vec4 b;
	vec4 c;
	vec4 d;
} push;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = cbo[1][2].a;
	FragColor += cbo[1][2].b;
	FragColor += cbo[1][2].c;
	FragColor += cbo[1][2].d;
	FragColor += push.a;
	FragColor += push.b;
	FragColor += push.c;
	FragColor += push.d;
}

