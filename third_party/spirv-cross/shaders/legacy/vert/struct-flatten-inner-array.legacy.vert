#version 450

struct Foo
{
	float a[4];
};

layout(location = 0) out Foo foo;

void main()
{
	gl_Position = vec4(1.0);
	for (int i = 0; i < 4; i++)
		foo.a[i] = float(i + 2);
}
