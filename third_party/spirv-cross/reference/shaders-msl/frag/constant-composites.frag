#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Foo
{
    float a;
    float b;
};

constant float _16[4] = {1.0, 4.0, 3.0, 2.0};
constant Foo _28[2] = {{10.0, 20.0}, {30.0, 40.0}};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int line [[user(locn0)]];
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
    float lut[4] = {1.0, 4.0, 3.0, 2.0};
    Foo foos[2] = {{10.0, 20.0}, {30.0, 40.0}};
    out.FragColor = float4(lut[in.line]);
    out.FragColor += float4(foos[in.line].a * foos[1 - in.line].a);
    return out;
}

