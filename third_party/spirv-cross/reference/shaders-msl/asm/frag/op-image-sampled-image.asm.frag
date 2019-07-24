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
    float4 r0;
    r0 = float4(_19.cb0[0u].z, _19.cb0[0u].w, r0.z, r0.w);
    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(-1, -2)), as_type<int>(r0.w));
    return out;
}

