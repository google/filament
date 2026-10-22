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

fragment main0_out main0(main0_in in [[stage_in]], device _7& _10 [[buffer(0)]], texture2d<float> _8 [[texture(0)]])
{
    main0_out out = {};
    for (int _154 = 0; _154 < 64; )
    {
        _10._m0[_154] = _8.read(uint2(int2(_154 - 8 * (_154 / 8), _154 / 8)), 0);
        _154++;
        continue;
    }
    out.m_3 = in.m_2;
    return out;
}

