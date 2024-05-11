#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 m_4 [[color(0)]];
};

struct main0_in
{
    float2 m_3 [[user(locn0)]];
};

static inline __attribute__((always_inline))
float4 _19(thread float2& _3, texture2d<float> _5, sampler _5Smplr)
{
    return _5.sample(_5Smplr, _3);
}

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> _5 [[texture(0)]], sampler _5Smplr [[sampler(0)]])
{
    main0_out out = {};
    out.m_4 = _19(in.m_3, _5, _5Smplr);
    return out;
}

