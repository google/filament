#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 m_6 [[color(0)]];
};

struct main0_in
{
    float2 m_4 [[user(locn0)]];
};

static inline __attribute__((always_inline))
float4 _22(thread float2& _4, texture2d<float> _7, sampler _7Smplr)
{
    return _7.sample(_7Smplr, _4);
}

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> _7 [[texture(0)]], sampler _7Smplr [[sampler(0)]])
{
    main0_out out = {};
    out.m_6 = _22(in.m_4, _7, _7Smplr);
    return out;
}

