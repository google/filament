#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor0 [[color(0), index(0)]];
    float4 FragColor1 [[color(0), index(1)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor0 = float4(1.0);
    out.FragColor1 = float4(2.0);
    return out;
}

