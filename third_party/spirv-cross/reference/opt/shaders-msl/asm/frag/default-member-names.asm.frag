#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float _49 = {};

struct main0_out
{
    float4 m_3 [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.m_3 = float4(_49);
    return out;
}

