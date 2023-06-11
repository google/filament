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
void _108(int _109, texture2d<float> _8, sampler _9, device _7& _10)
{
    _10._m0[_109] = _8.sample(_9, (float2(int2(_109 - 8 * (_109 / 8), _109 / 8)) / float2(8.0)), level(0.0));
}

static inline __attribute__((always_inline))
float4 _98(float4 _121, texture2d<float> _8, sampler _9, device _7& _10)
{
    for (int _123 = 0; _123 < 64; _123++)
    {
        _108(_123, _8, _9, _10);
    }
    return _121;
}

fragment main0_out main0(main0_in in [[stage_in]], device _7& _10 [[buffer(0)]], texture2d<float> _8 [[texture(0)]], sampler _9 [[sampler(0)]])
{
    main0_out out = {};
    float4 _97 = _98(in.m_2, _8, _9, _10);
    out.m_3 = _97;
    return out;
}

