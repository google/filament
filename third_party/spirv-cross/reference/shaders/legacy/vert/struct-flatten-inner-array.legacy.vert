#version 100

struct Foo
{
    float a[4];
};

varying float foo_a[4];

void main()
{
    gl_Position = vec4(1.0);
    for (int i = 0; i < 4; i++)
    {
        foo_a[i] = float(i + 2);
    }
}

