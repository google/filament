#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO1
{
    int4 c;
    int4 d;
};

struct UBO2
{
    uint4 e;
    uint4 f;
};

struct UBO0
{
    float4 a;
    float4 b;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant UBO0& _41 [[buffer(0)]], constant UBO1& _14 [[buffer(1)]], constant UBO2& _29 [[buffer(2)]])
{
    main0_out out = {};
    out.FragColor = ((((float4(_14.c) + float4(_14.d)) + float4(_29.e)) + float4(_29.f)) + _41.a) + _41.b;
    return out;
}

