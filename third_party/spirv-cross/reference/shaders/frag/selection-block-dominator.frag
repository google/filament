#version 450

layout(location = 0) flat in int vIndex;
layout(location = 0) out vec4 FragColor;

void main()
{
    int v;
    if (vIndex != 1)
    {
        FragColor = vec4(1.0);
        return;
    }
    else
    {
        v = 10;
    }
    FragColor = vec4(float(v));
}

