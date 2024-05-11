#version 450

struct VOUT
{
	vec4 a;
};

layout(location = 0) in VOUT Clip;
layout(location = 0) out vec4 FragColor;

void main()
{
	VOUT tmp = Clip;
	tmp.a += 1.0;
	FragColor = tmp.a;
}
