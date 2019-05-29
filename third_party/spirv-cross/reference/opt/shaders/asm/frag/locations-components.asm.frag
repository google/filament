#version 450

layout(location = 1) in vec2 _2;
layout(location = 1, component = 2) in float _3;
layout(location = 2) flat in float _4;
layout(location = 2, component = 1) flat in uint _5;
layout(location = 2, component = 2) flat in uint _6;
layout(location = 0) out vec4 o0;
vec4 v1;
vec4 v2;

void main()
{
    v1 = vec4(_2.x, _2.y, v1.z, v1.w);
    v1.z = _3;
    v2.x = _4;
    v2.y = uintBitsToFloat(_5);
    v2.z = uintBitsToFloat(_6);
    o0.y = float(floatBitsToUint(intBitsToFloat(floatBitsToInt(v2.y) + floatBitsToInt(v2.z))));
    o0.x = v1.y + v2.x;
    o0 = vec4(o0.x, o0.y, v1.z, v1.x);
}

