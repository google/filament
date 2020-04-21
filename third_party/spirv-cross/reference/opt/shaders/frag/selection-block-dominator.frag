#version 450

layout(location = 0) flat in int vIndex;
layout(location = 0) out vec4 FragColor;

void main()
{
    switch (0u)
    {
        default:
        {
            if (vIndex != 1)
            {
                FragColor = vec4(1.0);
                break;
            }
            FragColor = vec4(10.0);
            break;
        }
    }
}

