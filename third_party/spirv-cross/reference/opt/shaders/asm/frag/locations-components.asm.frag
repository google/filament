#version 450

layout(location = 1) in vec2 _8;
layout(location = 1, component = 2) in float _16;
layout(location = 2) flat in float _22;
layout(location = 2, component = 1) flat in uint _28;
layout(location = 2, component = 2) flat in uint _33;
layout(location = 0) out vec4 o0;
vec4 v1;
vec4 v2;

void main()
{
    v1 = vec4(_8.x, _8.y, v1.z, v1.w);
    v1.z = _16;
    v2.x = _22;
    v2.y = uintBitsToFloat(_28);
    v2.z = uintBitsToFloat(_33);
    o0.y = float(floatBitsToUint(intBitsToFloat(floatBitsToInt(v2.y) + floatBitsToInt(v2.z))));
    o0.x = v1.y + v2.x;
    o0 = vec4(o0.x, o0.y, v1.z, v1.x);
}

