#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, uint N>
inline void spvArrayCopyFromConstantToStack(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToThreadGroup(threadgroup T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToStack(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToThreadGroup(threadgroup T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToStack(thread T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToThreadGroup(threadgroup T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToDevice(device T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToDevice(device T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToDevice(device T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToDevice(device T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToStack(thread T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToThreadGroup(threadgroup T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

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

static inline __attribute__((always_inline))
void test(thread float4 (&spvReturnValue)[2])
{
    spvArrayCopyFromConstantToStack(spvReturnValue, _20);
}

static inline __attribute__((always_inline))
void test2(thread float4 (&spvReturnValue)[2], thread float4& vInput0, thread float4& vInput1)
{
    float4 foobar[2];
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    spvArrayCopyFromStackToStack(spvReturnValue, foobar);
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

