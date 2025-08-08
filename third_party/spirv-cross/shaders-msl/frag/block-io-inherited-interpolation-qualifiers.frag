#version 450

struct Foo
{
	vec2 M;
	float F;
};

layout(location = 0) in V { flat Foo foo; };
layout(location = 0) out float Fo;

void main()
{
	Fo = foo.F;
}
