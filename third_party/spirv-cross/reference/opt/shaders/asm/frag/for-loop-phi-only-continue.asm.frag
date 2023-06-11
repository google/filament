#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    float _19;
    _19 = 0.0;
    for (int _22 = 0; _22 < 16; )
    {
        _19 += 1.0;
        _22++;
        continue;
    }
    FragColor = vec4(_19);
}

