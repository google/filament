#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 color [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.color = float4(1.0);
    return out;
}

