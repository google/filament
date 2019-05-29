#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#ifndef SPIRV_CROSS_CONSTANT_ID_10
#define SPIRV_CROSS_CONSTANT_ID_10 2
#endif
constant int Value = SPIRV_CROSS_CONSTANT_ID_10;

struct SpecConstArray
{
    float4 samples[Value];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int Index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant SpecConstArray& _15 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = _15.samples[in.Index];
    return out;
}

