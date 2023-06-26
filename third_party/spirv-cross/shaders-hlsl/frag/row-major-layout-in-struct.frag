#version 450

struct Foo
{
	mat4 v;
	mat4 w;
};

struct NonFoo
{
	mat4 v;
	mat4 w;
};

layout(std140, binding = 0) uniform UBO
{
	layout(column_major) Foo foo;
};

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vUV;

void main()
{
	NonFoo f;
	f.v = foo.v;
	f.w = foo.w;
	FragColor = f.v * (f.w * vUV);
}
