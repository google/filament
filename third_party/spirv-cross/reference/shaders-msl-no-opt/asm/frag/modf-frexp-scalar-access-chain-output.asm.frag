#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    float3 col;
    int2 _18;
    float _23;
    float _21 = modf(0.1500000059604644775390625, _23);
    col.x = _23;
    int _24;
    float _22 = frexp(0.1500000059604644775390625, _24);
    _18.y = _24;
}

