#version 450

#ifndef SPIRV_CROSS_CONSTANT_ID_201
#define SPIRV_CROSS_CONSTANT_ID_201 -10
#endif
const int _7 = SPIRV_CROSS_CONSTANT_ID_201;
const int _20 = (_7 + 2);
#ifndef SPIRV_CROSS_CONSTANT_ID_202
#define SPIRV_CROSS_CONSTANT_ID_202 100u
#endif
const uint _8 = SPIRV_CROSS_CONSTANT_ID_202;
const uint _25 = (_8 % 5u);
const int _30 = _7 - (-3) * (_7 / (-3));
const ivec4 _32 = ivec4(20, 30, _20, _30);
const ivec2 _34 = ivec2(_32.y, _32.x);
const int _35 = _32.y;

layout(location = 0) flat out int _4;

void main()
{
    vec4 _65 = vec4(0.0);
    _65.y = float(_20);
    _65.z = float(_25);
    vec4 _54 = _65 + vec4(_32);
    vec2 _58 = _54.xy + vec2(_34);
    gl_Position = vec4(_58.x, _58.y, _54.z, _54.w);
    _4 = _35;
}

