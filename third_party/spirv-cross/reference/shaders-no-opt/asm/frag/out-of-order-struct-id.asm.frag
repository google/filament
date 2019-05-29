#version 450

struct Foo
{
    vec4 a;
};

struct Bar
{
    Foo foo;
    Foo foo2;
};

layout(binding = 0, std140) uniform UBO
{
    Bar bar;
} _7;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = _7.bar.foo.a + _7.bar.foo2.a;
}

