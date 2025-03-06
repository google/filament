#version 450

layout(binding = 0, std140) uniform UBO
{
    int uCount;
    int uJ;
    int uK;
} _7;

layout(location = 0) out float FragColor;

void main()
{
    int _53;
    int _54;
    int _54_copy;
    _54 = _7.uK;
    _53 = _7.uJ;
    for (int _52 = 0; _52 < _7.uCount; _54_copy = _54, _54 = _53, _53 = _54_copy, _52++)
    {
    }
    FragColor = float(_53 - _54) * float(_7.uJ * _7.uK);
}

