#version 450

layout(location = 0) out float FragColor;

struct T
{
	float a;
};

void main()
{
	T foo;
	struct T { float b; };
	T bar;

	foo.a = 10.0;
	bar.b = 20.0;
	FragColor = foo.a + bar.b;
}
