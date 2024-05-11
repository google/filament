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

layout(location = 0) out VertexIn
{
	Foo a;
	Bar b;
};

layout(location = 4) out Baz baz;

void main()
{
	a.a = vec4(10.0);
	a.b = vec4(20.0);
	b.a = vec4(30.0);
	b.b = vec4(40.0);
	a = Foo(vec4(50.0), vec4(60.0));
	b = Bar(vec4(50.0), vec4(60.0));
	baz.foo = Foo(vec4(100.0), vec4(200.0));
	baz.bar = Bar(vec4(300.0), vec4(400.0));
	baz = Baz(Foo(vec4(1000.0), vec4(2000.0)), Bar(vec4(3000.0), vec4(4000.0)));
}
