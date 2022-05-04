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
void _108(int _109, texture2d<float> v_8, device _7& v_10)
{
    int2 _113 = int2(_109 - 8 * (_109 / 8), _109 / 8);
    v_10._m0[_109] = v_8.read(uint2(_113), 0);
}

static inline __attribute__((always_inline))
float4 _98(float4 _119, texture2d<float> v_8, device _7& v_10)
{
    for (int _121 = 0; _121 < 64; _121++)
    {
        _108(_121, v_8, v_10);
    }
    return _119;
}

fragment main0_out main0(main0_in in [[stage_in]], device _7& v_10 [[buffer(0)]], texture2d<float> v_8 [[texture(0)]])
{
    main0_out out = {};
    float4 _97 = _98(in.m_2, v_8, v_10);
    out.m_3 = _97;
    return out;
}

