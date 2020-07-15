#version 450

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 FragColor;

int uninit_int;
ivec4 uninit_vector;
mat4 uninit_matrix;

struct Foo { int a; };
Foo uninit_foo;

void main()
{
	int uninit_function_int;
	if (vColor.x > 10.0)
		uninit_function_int = 10;
	else
		uninit_function_int = 20;
	FragColor = vColor;
}
