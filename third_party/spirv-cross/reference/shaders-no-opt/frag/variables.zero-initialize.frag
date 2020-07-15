#version 450

struct Foo
{
    int a;
};

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 FragColor;
int uninit_int = 0;
ivec4 uninit_vector = ivec4(0);
mat4 uninit_matrix = mat4(vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));
Foo uninit_foo = Foo(0);

void main()
{
    int uninit_function_int = 0;
    if (vColor.x > 10.0)
    {
        uninit_function_int = 10;
    }
    else
    {
        uninit_function_int = 20;
    }
    FragColor = vColor;
}

