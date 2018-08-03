#version 450

layout(binding = 13, std140) uniform _1365_18812
{
    layout(row_major) mat4x3 _m0;
    layout(row_major) mat4x3 _m1;
} _18812;

layout(binding = 12, std140) uniform _1126_22044
{
    layout(row_major) mat4 _m0;
    layout(row_major) mat4 _m1;
    float _m9;
    vec3 _m10;
    float _m11;
    vec3 _m12;
    float _m17;
    float _m18;
    float _m19;
    vec2 _m20;
} _22044;

layout(location = 0) out vec3 _3976;
layout(location = 0) in vec4 _5275;

vec3 _2;

void main()
{
    vec3 _23783;
    do
    {
        _23783 = normalize(_18812._m1 * vec4(_5275.xyz, 0.0));
        break;
    } while (false);
    vec4 _14995 = vec4(_22044._m10 + (_5275.xyz * (_22044._m17 + _22044._m18)), 1.0) * _22044._m0;
    _3976 = _23783;
    vec4 _6282 = _14995;
    _6282.y = -_14995.y;
    gl_Position = _6282;
}
