#version 450

layout(binding = 0, std430) readonly buffer SSBO
{
    float values0[];
} _5;

layout(binding = 1, std430) readonly buffer SSBO1
{
    float values1[];
} _7;

layout(location = 0) out vec2 FragColor;

void main()
{
    vec2 _27;
    _27 = vec2(0.0);
    vec2 _42;
    for (int _30 = 0; _30 < 16; _27 += _42, _30++)
    {
        vec2 _40 = _27 * _27;
        _40.x = _5.values0[_30];
        _42 = _40;
        _42.y = _7.values1[_30];
    }
    FragColor = _27;
}

