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
void _112(int _113, texture2d<float> _8, device _7& _10)
{
    int2 _117 = int2(_113 - 8 * (_113 / 8), _113 / 8);
    _10._m0[_113] = _8.read(uint2(_117), 0);
}

static inline __attribute__((always_inline))
float4 _102(float4 _122, texture2d<float> _8, device _7& _10)
{
    for (int _124 = 0; _124 < 64; _124++)
    {
        _112(_124, _8, _10);
    }
    return _122;
}

fragment main0_out main0(main0_in in [[stage_in]], device _7& _10 [[buffer(0)]], texture2d<float> _8 [[texture(0)]])
{
    main0_out out = {};
    float4 _101 = _102(in.m_2, _8, _10);
    out.m_3 = _101;
    return out;
}

