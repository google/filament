#version 450

layout(location = 0) out vec4 FragColor;

void main()
{
    float _50;
    _50 = 0.0;
    float _25;
    int _28;
    for (int _47 = 0; _47 < 16; _50 = _25, _47 = _28)
    {
        _25 = _50 + 1.0;
        _28 = _47 + 1;
    }
    FragColor = vec4(_50);
}

