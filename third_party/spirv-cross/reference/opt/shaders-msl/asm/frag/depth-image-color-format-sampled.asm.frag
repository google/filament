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

fragment main0_out main0(main0_in in [[stage_in]], device _7& _10 [[buffer(0)]], texture2d<float> _8 [[texture(0)]], sampler _9 [[sampler(0)]])
{
    main0_out out = {};
    for (int _158 = 0; _158 < 64; )
    {
        _10._m0[_158] = _8.sample(_9, (float2(int2(_158 - 8 * (_158 / 8), _158 / 8)) * float2(0.125)), level(0.0));
        _158++;
        continue;
    }
    out.m_3 = in.m_2;
    return out;
}

