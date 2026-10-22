#version 450

struct Foo
{
	vec4 bar[2];
	vec4 baz[2];
};

layout(location = 0) out Vertex
{
	Foo foo;
	Foo foo2;
};

layout(location = 8) out Foo foo3;

void main()
{
	foo.bar[0] = vec4(1.0);
	foo.baz[1] = vec4(2.0);
	foo2.bar[0] = vec4(3.0);
	foo2.baz[1] = vec4(4.0);
	foo3.bar[0] = vec4(5.0);
	foo3.baz[1] = vec4(6.0);
}
