#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float4 fragment0 = float4(10.0);
    float4 compute0 = float4(10.0);
    float4 kernel0 = float4(10.0);
    float4 vertex0 = float4(10.0);
    out.FragColor = ((fragment0 + compute0) + kernel0) + vertex0;
    return out;
}

