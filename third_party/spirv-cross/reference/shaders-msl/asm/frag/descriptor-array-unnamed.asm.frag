#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _4
{
    float4 _m0;
};

struct _6
{
    int _m0;
};

struct _7
{
    float4 _m0;
};

struct main0_out
{
    float4 m_3 [[color(0)]];
};

fragment main0_out main0(const device _4* _5_0 [[buffer(0)]], const device _4* _5_1 [[buffer(1)]], const device _4* _5_2 [[buffer(2)]], const device _4* _5_3 [[buffer(3)]], constant _6& _20 [[buffer(4)]], constant _7* _8_0 [[buffer(5)]], constant _7* _8_1 [[buffer(6)]], constant _7* _8_2 [[buffer(7)]], constant _7* _8_3 [[buffer(8)]])
{
    const device _4* _5[] =
    {
        _5_0,
        _5_1,
        _5_2,
        _5_3,
    };

    constant _7* _8[] =
    {
        _8_0,
        _8_1,
        _8_2,
        _8_3,
    };

    main0_out out = {};
    out.m_3 = _5[_20._m0]->_m0 + (_8[_20._m0]->_m0 * float4(0.20000000298023223876953125));
    return out;
}

