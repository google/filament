#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _6
{
    float4 _m0;
};

struct _8
{
    int _m0;
};

struct _9
{
    float4 _m0;
};

struct main0_out
{
    float4 m_4 [[color(0)]];
};

fragment main0_out main0(const device _6* _7_0 [[buffer(0)]], const device _6* _7_1 [[buffer(1)]], const device _6* _7_2 [[buffer(2)]], const device _6* _7_3 [[buffer(3)]], constant _8& _21 [[buffer(4)]], constant _9* _10_0 [[buffer(5)]], constant _9* _10_1 [[buffer(6)]], constant _9* _10_2 [[buffer(7)]], constant _9* _10_3 [[buffer(8)]])
{
    const device _6* _7[] =
    {
        _7_0,
        _7_1,
        _7_2,
        _7_3,
    };

    constant _9* _10[] =
    {
        _10_0,
        _10_1,
        _10_2,
        _10_3,
    };

    main0_out out = {};
    out.m_4 = _7[_21._m0]->_m0 + (_10[_21._m0]->_m0 * float4(0.20000000298023223876953125));
    return out;
}

