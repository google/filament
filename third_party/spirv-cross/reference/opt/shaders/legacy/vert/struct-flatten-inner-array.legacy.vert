#version 100

struct Foo
{
    float a[4];
};

varying float foo_a[4];

void main()
{
    gl_Position = vec4(1.0);
    for (int _46 = 0; _46 < 4; foo_a[_46] = float(_46 + 2), _46++)
    {
    }
}

