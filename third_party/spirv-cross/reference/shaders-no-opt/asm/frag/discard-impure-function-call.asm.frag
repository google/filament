#version 450

layout(location = 0) flat in int vA;
layout(location = 0) out vec4 FragColor;

vec4 foobar(int a)
{
    if (a < 0)
    {
        discard;
    }
    return vec4(10.0);
}

void main()
{
    int param = vA;
    vec4 _25 = foobar(param);
    FragColor = vec4(10.0);
}

