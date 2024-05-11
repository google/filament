#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _9
{
    float _m0;
};

struct _10
{
    float _m0;
    float _m1;
    float _m2;
    float _m3;
    float _m4;
    float _m5;
    float _m6;
    float _m7;
    float _m8;
    float _m9;
    float _m10;
    float _m11;
    _9 _m12;
};

struct main0_out
{
    float4 m_3 [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    _10 _21;
    out.m_3 = float4(_21._m0, _21._m1, _21._m2, _21._m3);
    return out;
}

