#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Foobar
{
    float a;
    float b;
};

constant float4 _37[3] = {float4(1.0), float4(2.0), float4(3.0)};
constant float4 _49[2] = {float4(1.0), float4(2.0)};
constant float4 _54[2] = {float4(8.0), float4(10.0)};
constant float4 _55[2][2] = {{float4(1.0), float4(2.0)}, {float4(8.0), float4(10.0)}};
constant Foobar _75[2] = {{10.0, 40.0}, {90.0, 70.0}};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

// Implementation of an array copy function to cover GLSL's ability to copy an array via assignment.
template<typename T, uint N>
void spvArrayCopy(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

// An overload for constant arrays.
template<typename T, uint N>
void spvArrayCopyConstant(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Foobar indexable[2] = {{10.0, 40.0}, {90.0, 70.0}};
    out.FragColor = ((_37[in.index] + _55[in.index][in.index + 1]) + float4(30.0)) + float4(indexable[in.index].a + indexable[in.index].b);
    return out;
}

