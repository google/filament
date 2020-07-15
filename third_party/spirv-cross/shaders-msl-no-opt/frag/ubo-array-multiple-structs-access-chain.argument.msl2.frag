#version 450

struct Foo
{
	vec4 v;
};

layout(set = 0, binding = 0) uniform UBO
{
	Foo foo;
} ubos[2];

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = ubos[1].foo.v;
}
