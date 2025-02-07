#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _7
{
    float4 _m0[64];
};

struct main0_out
{
    float4 m_3 [[color(0)]];
};

struct main0_in
{
    float4 m_2 [[user(locn1)]];
};

static inline __attribute__((always_inline))
void _112(int _113, texture2d<float> _8, sampler _9, device _7& _10)
{
    _10._m0[_113] = _8.sample(_9, (float2(int2(_113 - 8 * (_113 / 8), _113 / 8)) / float2(8.0)), level(0.0));
}

static inline __attribute__((always_inline))
float4 _102(float4 _124, texture2d<float> _8, sampler _9, device _7& _10)
{
    for (int _126 = 0; _126 < 64; _126++)
    {
        _112(_126, _8, _9, _10);
    }
    return _124;
}

fragment main0_out main0(main0_in in [[stage_in]], device _7& _10 [[buffer(0)]], texture2d<float> _8 [[texture(0)]], sampler _9 [[sampler(0)]])
{
    main0_out out = {};
    float4 _101 = _102(in.m_2, _8, _9, _10);
    out.m_3 = _101;
    return out;
}

