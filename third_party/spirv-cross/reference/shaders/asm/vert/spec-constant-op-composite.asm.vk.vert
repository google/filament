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
#ifndef SPIRV_CROSS_CONSTANT_ID_200
#define SPIRV_CROSS_CONSTANT_ID_200 3.141590118408203125
#endif
const float _57 = SPIRV_CROSS_CONSTANT_ID_200;

layout(location = 0) flat out int _58;

void main()
{
    vec4 pos = vec4(0.0);
    pos.y += float(_15);
    pos.z += float(_26);
    pos += vec4(_36);
    vec2 _46 = pos.xy + vec2(_41);
    pos = vec4(_46.x, _46.y, pos.z, pos.w);
    gl_Position = pos;
    _58 = _62;
}

