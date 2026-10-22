#version 450

struct Foo
{
	vec4 a;
	vec4 b;
};

struct Bar
{
	vec4 a;
	vec4 b;
};

struct Baz
{
	Foo foo;
	Bar bar;
};

layout(location = 0) in VertexIn
{
	Foo a;
	Bar b;
};

layout(location = 4) in Baz baz;

layout(location = 0) out vec4 FragColor;

void main()
{
	Baz bazzy = baz;
	Foo bazzy_foo = baz.foo;
	Bar bazzy_bar = baz.bar;
	FragColor = a.a + b.b + bazzy.foo.b + bazzy_foo.a + bazzy_bar.b;
}
