#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 v0;

void main()
{
    FragColor = vec4(1.0);
    for (int _54 = 0; _54 < 4; _54++)
    {
        if (v0.x == 20.0)
        {
            FragColor += vec4(v0[_54 & 3]);
            continue;
        }
        else
        {
            FragColor += vec4(v0[_54 & 1]);
            continue;
        }
        continue;
    }
}

