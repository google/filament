#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _5ma_in_out
{
    float4 FragColor [[color(0)]];
};

fragment _5ma_in_out _5ma_in()
{
    _5ma_in_out out = {};
    out.FragColor = float4(1.0);
    return out;
}

