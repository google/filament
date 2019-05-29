#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float4 _20[2] = { float4(1.0, 2.0, 3.0, 4.0), float4(10.0) };

struct main0_out
{
    float4 FragColors_0 [[color(0)]];
    float4 FragColors_1 [[color(1)]];
    float4 FragColor [[color(2)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float4 FragColors[2] = { float4(1.0, 2.0, 3.0, 4.0), float4(10.0) };
    out.FragColor = float4(5.0);
    out.FragColors_0 = FragColors[0];
    out.FragColors_1 = FragColors[1];
    return out;
}

