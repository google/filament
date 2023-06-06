#version 450

struct T
{
    float a;
};

layout(location = 0) out float FragColor;

void main()
{
    T foo;
    foo.a = 10.0;
    T bar;
    bar.a = 20.0;
    FragColor = foo.a + bar.a;
}

