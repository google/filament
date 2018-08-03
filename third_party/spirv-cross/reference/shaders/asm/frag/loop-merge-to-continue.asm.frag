#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 v0;

void main()
{
    FragColor = vec4(1.0);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            FragColor += vec4(v0[(i + j) & 3]);
        }
    }
}

