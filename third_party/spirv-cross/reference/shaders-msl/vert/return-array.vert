#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float4 _20[2] = { float4(10.0), float4(20.0) };

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vInput0 [[attribute(0)]];
    float4 vInput1 [[attribute(1)]];
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

void test(thread float4 (&SPIRV_Cross_return_value)[2])
{
    spvArrayCopyFromConstant1(SPIRV_Cross_return_value, _20);
}

void test2(thread float4 (&SPIRV_Cross_return_value)[2], thread float4& vInput0, thread float4& vInput1)
{
    float4 foobar[2];
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    spvArrayCopyFromStack1(SPIRV_Cross_return_value, foobar);
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 _42[2];
    test(_42);
    float4 _44[2];
    test2(_44, in.vInput0, in.vInput1);
    out.gl_Position = _42[0] + _44[1];
    return out;
}

