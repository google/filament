#version 450

struct Foo
{
	float a;
	float b;
};

layout(location = 1) in Foo foos[4];
layout(location = 10) in Foo bars[4];
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor.x = foos[0].a;
	FragColor.y = foos[1].b;
	FragColor.z = foos[2].a;
	FragColor.w = bars[3].b.x;
}