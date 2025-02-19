#version 450

#ifndef SPIRV_CROSS_CONSTANT_ID_201
#define SPIRV_CROSS_CONSTANT_ID_201 -10
#endif
const int _13 = SPIRV_CROSS_CONSTANT_ID_201;
const int _15 = (_13 + 2);
#ifndef SPIRV_CROSS_CONSTANT_ID_202
#define SPIRV_CROSS_CONSTANT_ID_202 100u
#endif
const uint _24 = SPIRV_CROSS_CONSTANT_ID_202;
const uint _26 = (_24 % 5u);
const int _61 = _13 - (-3) * (_13 / (-3));
const ivec4 _36 = ivec4(20, 30, _15, _61);
const ivec2 _41 = ivec2(_36.y, _36.x);
const int _62 = _36.y;

layout(location = 0) flat out int _58;

void main()
{
    vec4 _65 = vec4(0.0);
    _65.y = float(_15);
    _65.z = float(_26);
    vec4 _39 = _65 + vec4(_36);
    vec2 _46 = _39.xy + vec2(_41);
    gl_Position = vec4(_46.x, _46.y, _39.z, _39.w);
    _58 = _62;
}

