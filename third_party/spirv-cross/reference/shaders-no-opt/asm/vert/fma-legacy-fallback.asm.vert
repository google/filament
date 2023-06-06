#version 310 es

layout(location = 0) out float a;

void main()
{
    a = 5.0;
    a = a * 2.0 + 1.0;
}

