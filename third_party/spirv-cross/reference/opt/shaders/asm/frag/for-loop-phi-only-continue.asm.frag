#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    float _50;
    _50 = 0.0;
    for (int _47 = 0; _47 < 16; )
    {
        _50 += 1.0;
        _47++;
        continue;
    }
    FragColor = vec4(_50);
}

