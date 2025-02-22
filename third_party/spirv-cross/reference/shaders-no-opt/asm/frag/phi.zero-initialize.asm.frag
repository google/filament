#version 450

int uninit_int = 0;
ivec4 uninit_vector = ivec4(0);
mat4 uninit_matrix = mat4(vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0));

struct Foo
{
    int a;
};

Foo uninit_foo = Foo(0);

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 FragColor;

void main()
{
    int _27 = 0;
    if (vColor.x > 10.0)
    {
        _27 = 10;
    }
    else
    {
        _27 = 20;
    }
    FragColor = vColor;
}

