#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 v0;

void main()
{
    FragColor = vec4(1.0);
    for (int i = 0; i < 4; i++)
    {
        if (v0.x == 20.0)
        {
            FragColor += vec4(v0[i & 3]);
        }
        else
        {
            FragColor += vec4(v0[i & 1]);
        }
    }
}

