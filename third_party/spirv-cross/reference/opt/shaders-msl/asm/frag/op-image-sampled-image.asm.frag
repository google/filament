#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct push_cb
{
    float4 cb0[1];
};

struct main0_out
{
    float4 o0 [[color(0)]];
};

fragment main0_out main0(constant push_cb& _19 [[buffer(0)]], texture2d<float> t0 [[texture(0)]], sampler dummy_sampler [[sampler(0)]])
{
    main0_out out = {};
    out.o0 = t0.read(uint2(as_type<int2>(_19.cb0[0u].zw)) + uint2(int2(-1, -2)), as_type<int>(0.0));
    return out;
}

