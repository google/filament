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

template<typename T, uint A>
inline void spvArrayCopyFromConstantToStack1(thread T (&dst)[A], constant T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromConstantToThreadGroup1(threadgroup T (&dst)[A], constant T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromStackToStack1(thread T (&dst)[A], thread const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromStackToThreadGroup1(threadgroup T (&dst)[A], thread const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromThreadGroupToStack1(thread T (&dst)[A], threadgroup const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromThreadGroupToThreadGroup1(threadgroup T (&dst)[A], threadgroup const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromDeviceToDevice1(device T (&dst)[A], device const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromConstantToDevice1(device T (&dst)[A], constant T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromStackToDevice1(device T (&dst)[A], thread const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromThreadGroupToDevice1(device T (&dst)[A], threadgroup const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromDeviceToStack1(thread T (&dst)[A], device const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint A>
inline void spvArrayCopyFromDeviceToThreadGroup1(threadgroup T (&dst)[A], device const T (&src)[A])
{
    for (uint i = 0; i < A; i++)
    {
        dst[i] = src[i];
    }
}

static inline __attribute__((always_inline))
void test(thread float4 (&SPIRV_Cross_return_value)[2])
{
    spvArrayCopyFromConstantToStack1(SPIRV_Cross_return_value, _20);
}

static inline __attribute__((always_inline))
void test2(thread float4 (&SPIRV_Cross_return_value)[2], thread float4& vInput0, thread float4& vInput1)
{
    float4 foobar[2];
    foobar[0] = vInput0;
    foobar[1] = vInput1;
    spvArrayCopyFromStackToStack1(SPIRV_Cross_return_value, foobar);
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

