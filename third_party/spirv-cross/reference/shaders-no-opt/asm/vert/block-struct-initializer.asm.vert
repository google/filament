#version 450

struct Foo
{
    float c;
    float d;
};

layout(location = 0) out Vert
{
    float a;
    float b;
} _4;

layout(location = 2) out Foo foo;
const Foo _6_init = Foo(0.0, 0.0);

void main()
{
    _4.a = 0.0;
    _4.b = 0.0;
    foo = _6_init;
}

