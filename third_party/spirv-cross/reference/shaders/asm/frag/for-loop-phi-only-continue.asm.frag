#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    float _19;
    _19 = 0.0;
    float _20;
    int _23;
    for (int _22 = 0; _22 < 16; _19 = _20, _22 = _23)
    {
        _20 = _19 + 1.0;
        _23 = _22 + 1;
        continue;
    }
    FragColor = vec4(_19);
}

