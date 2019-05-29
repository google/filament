#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float4 _68[4] = { float4(0.0), float4(1.0), float4(2.0), float4(3.0) };

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    int Index1 [[attribute(0)]];
    int Index2 [[attribute(1)]];
};

// Implementation of an array copy function to cover GLSL's ability to copy an array via assignment.
template<typename T, uint N>
void spvArrayCopyFromStack1(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

template<typename T, uint N>
void spvArrayCopyFromConstant1(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

float4 consume_constant_arrays2(thread const float4 (&positions)[4], thread const float4 (&positions2)[4], thread int& Index1, thread int& Index2)
{
    float4 indexable[4];
    spvArrayCopyFromStack1(indexable, positions);
    float4 indexable_1[4];
    spvArrayCopyFromStack1(indexable_1, positions2);
    return indexable[Index1] + indexable_1[Index2];
}

float4 consume_constant_arrays(thread const float4 (&positions)[4], thread const float4 (&positions2)[4], thread int& Index1, thread int& Index2)
{
    return consume_constant_arrays2(positions, positions2, Index1, Index2);
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    float4 _68_array_copy[4] = { float4(0.0), float4(1.0), float4(2.0), float4(3.0) };
    main0_out out = {};
    float4 LUT2[4];
    LUT2[0] = float4(10.0);
    LUT2[1] = float4(11.0);
    LUT2[2] = float4(12.0);
    LUT2[3] = float4(13.0);
    out.gl_Position = consume_constant_arrays(_68_array_copy, LUT2, in.Index1, in.Index2);
    return out;
}

