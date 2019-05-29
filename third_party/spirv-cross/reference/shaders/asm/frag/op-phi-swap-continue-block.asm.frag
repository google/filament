#version 450

layout(binding = 0, std140) uniform UBO
{
    int uCount;
    int uJ;
    int uK;
} _5;

layout(location = 0) out float FragColor;

void main()
{
    int _23;
    int _23_copy;
    int _24;
    _23 = _5.uK;
    _24 = _5.uJ;
    for (int _26 = 0; _26 < _5.uCount; _23_copy = _23, _23 = _24, _24 = _23_copy, _26++)
    {
        continue;
    }
    FragColor = float(_24 - _23) * float(_5.uJ * _5.uK);
}

