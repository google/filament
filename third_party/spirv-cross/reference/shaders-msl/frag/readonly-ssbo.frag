#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SSBO
{
    float4 v;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

float4 read_from_function(const device SSBO& v_13)
{
    return v_13.v;
}

fragment main0_out main0(const device SSBO& v_13 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = v_13.v + read_from_function(v_13);
    return out;
}

