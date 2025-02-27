#version 450

layout(binding = 0, std430) readonly buffer SSBO
{
    float values0[];
} _7;

layout(binding = 1, std430) readonly buffer SSBO1
{
    float values1[];
} _9;

layout(location = 0) out vec2 FragColor;

void main()
{
    vec2 _61;
    _61 = vec2(0.0);
    vec2 _34;
    vec2 _35;
    vec2 _36;
    for (int _60 = 0; _60 < 16; _34 = _61 * _61, _35 = _34, _35.x = _7.values0[_60], _36 = _35, _36.y = _9.values1[_60], _61 += _36, _60++)
    {
    }
    FragColor = _61;
}

