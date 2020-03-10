#version 450

struct T
{
    float a;
};

struct T_1
{
    float b;
};

layout(location = 0) out float FragColor;

void main()
{
    T foo;
    foo.a = 10.0;
    T_1 bar;
    bar.b = 20.0;
    FragColor = foo.a + bar.b;
}

